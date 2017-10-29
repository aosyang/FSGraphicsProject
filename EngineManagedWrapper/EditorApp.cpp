//=============================================================================
// EditorApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Stdafx.h"
#include "EditorApp.h"

namespace EngineManagedWrapper
{
#pragma unmanaged
	EditorApp::EditorApp()
	{

	}

	EditorApp::~EditorApp()
	{
		m_Skybox.Release();
		m_Scene.Release();

		SAFE_RELEASE(m_SamplerState);
		SAFE_RELEASE(m_SamplerComparisonState);

		m_EditorAxis.Release();
	}

	bool EditorApp::Initialize()
	{
		GEngine.SetEditorMode(true);

		m_Scene.Initialize();

		m_DefaultShader = RShaderManager::Instance().GetShaderResource("Default");
		m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");

#if 1
		RResourceManager::Instance().LoadAllResources();
#else
		RResourceManager::Instance().LoadFbxMesh("../Assets/Sphere.fbx");
		RResourceManager::Instance().LoadFbxMesh("../Assets/SpeedballPlayer.fbx");
		RResourceManager::Instance().LoadFbxMesh("../Assets/AO_Scene.fbx");
		RResourceManager::Instance().LoadFbxMesh("../Assets/tachikoma.fbx");
		RResourceManager::Instance().LoadFbxMesh("../Assets/Island.fbx");
		RResourceManager::Instance().LoadFbxMesh("../Assets/city.fbx");
#endif

		m_Skybox.CreateSkybox("../Assets/powderpeak.dds");

		// Create texture sampler state
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		GRenderer.D3DDevice()->CreateSamplerState(&samplerDesc, &m_SamplerState);

		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		GRenderer.D3DDevice()->CreateSamplerState(&samplerDesc, &m_SamplerComparisonState);


		m_CamFov = 65.0f;
		m_CamYaw = m_CamPitch = 0.0f;
		m_CameraMatrix = RMatrix4::IDENTITY;
		m_MeshPos = RVec3::Zero();
		m_SelectedObject = nullptr;
		m_MouseControlMode = MouseControlMode::None;

		m_EditorAxis.Create();

		return true;
	}

	void EditorApp::UpdateScene(const RTimer& timer)
	{
		RVec3 moveVec(0.0f, 0.0f, 0.0f);

		RECT rwRect = GEngine.GetWindowRectInfo();
		int mx, my;
		RInput.GetCursorPos(mx, my);

		if (mx >= rwRect.left && mx <= rwRect.right &&
			my >= rwRect.top && my <= rwRect.bottom)
		{
			if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Pressed)
			{
				RInput.HideCursor();
				RInput.LockCursor();
			}

			if (RInput.GetBufferedKeyState(VK_LBUTTON) == BKS_Pressed)
			{
				RECT rect = GEngine.GetWindowRectInfo();
				int cur_x, cur_y;

				RInput.GetCursorPos(cur_x, cur_y);
				float fx = float(cur_x - rect.left) / float(rect.right - rect.left),
					fy = float(cur_y - rect.top) / float(rect.bottom - rect.top);

				RunScreenToCameraRayPicking(fx, fy);
			}

			if (RInput.IsKeyDown(VK_RBUTTON))
			{
				int dx, dy;
				RInput.GetCursorRelPos(dx, dy);
				if (dx || dy)
				{
					m_CamYaw += (float)dx / 200.0f;
					m_CamPitch += (float)dy / 200.0f;
					m_CamPitch = max(-PI / 2, min(PI / 2, m_CamPitch));
				}


				float camSpeed = 100.0f;
				if (RInput.IsKeyDown(VK_LSHIFT))
					camSpeed *= 10.0f;

				if (RInput.IsKeyDown('W'))
					moveVec += RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
				if (RInput.IsKeyDown('S'))
					moveVec -= RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
				if (RInput.IsKeyDown('A'))
					moveVec -= RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
				if (RInput.IsKeyDown('D'))
					moveVec += RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
			}
		}

		if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Released)
		{
			RInput.ShowCursor();
			RInput.UnlockCursor();
		}

		if (RInput.GetBufferedKeyState(VK_LBUTTON) == BKS_Released)
		{
			m_MouseControlMode = MouseControlMode::None;
		}

		if (m_MouseControlMode != MouseControlMode::None)
		{
			if (m_SelectedObject)
			{
				int mdx, mdy;
				RInput.GetCursorRelPos(mdx, mdy);

				RVec3 world_x = RVec3(1, 0, 0);
				RVec3 world_y = RVec3(0, 1, 0);
				RVec3 world_z = RVec3(0, 0, 1);

				RVec3 cam_right = m_CameraMatrix.GetRight();
				RVec3 cam_up = m_CameraMatrix.GetUp();

				float move_scale = GRenderer.GetClientHeight() / 500.0f;

				if (RInput.GetBufferedKeyState(VK_LBUTTON) == BKS_Pressed &&
					RInput.IsKeyDown(VK_LCONTROL))
				{
					m_SelectedObject = m_SelectedObject->Clone();
				}

				RVec3 pos = m_SelectedObject->GetPosition();

				if (m_MouseControlMode == MouseControlMode::MoveX)
				{
					if (RInput.IsKeyDown(VK_LMENU))
					{
						RMatrix4 transform = m_SelectedObject->GetTransformMatrix();
						RVec3 pos = transform.GetTranslation();
						transform.SetTranslation(RVec3(0, 0, 0));
						transform *= RMatrix4::CreateXAxisRotation((float)mdx);
						transform.SetTranslation(pos);
						m_SelectedObject->SetTransform(transform);
					}
					else
					{
						pos.SetX(pos.X() + (RVec3::Dot(world_x, cam_right) * mdx - RVec3::Dot(world_x, cam_up) * mdy) * move_scale);
					}
				}
				else if (m_MouseControlMode == MouseControlMode::MoveY)
				{
					if (RInput.IsKeyDown(VK_LMENU))
					{
						RMatrix4 transform = m_SelectedObject->GetTransformMatrix();
						RVec3 pos = transform.GetTranslation();
						transform.SetTranslation(RVec3(0, 0, 0));
						transform *= RMatrix4::CreateYAxisRotation((float)mdx);
						transform.SetTranslation(pos);
						m_SelectedObject->SetTransform(transform);
					}
					else
					{
						pos.SetY(pos.Y() + (RVec3::Dot(world_y, cam_right) * mdx - RVec3::Dot(world_y, cam_up) * mdy) * move_scale);
					}
				}
				else if (m_MouseControlMode == MouseControlMode::MoveZ)
				{
					if (RInput.IsKeyDown(VK_LMENU))
					{
						RMatrix4 transform = m_SelectedObject->GetTransformMatrix();
						RVec3 pos = transform.GetTranslation();
						transform.SetTranslation(RVec3(0, 0, 0));
						transform *= RMatrix4::CreateZAxisRotation((float)mdx);
						transform.SetTranslation(pos);
						m_SelectedObject->SetTransform(transform);
					}
					else
					{
						pos.SetZ(pos.Z() + (RVec3::Dot(world_z, cam_right) * mdx - RVec3::Dot(world_z, cam_up) * mdy) * move_scale);
					}
				}
				m_SelectedObject->SetPosition(pos);
			}
		}

		RVec3 camPos = m_CameraMatrix.GetTranslation();
		m_CameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);
		m_CameraMatrix.SetTranslation(camPos + (RVec4(moveVec, 1.0f) * m_CameraMatrix).ToVec3());

		RMatrix4 viewMatrix = m_CameraMatrix.FastInverse();
		RMatrix4 projMatrix = RMatrix4::CreatePerspectiveProjectionLH(m_CamFov, GRenderer.AspectRatio(), 1.0f, 10000.0f);
		RMatrix4 viewProjMatrix = viewMatrix * projMatrix;

		m_InvViewProjMatrix = viewProjMatrix.Inverse();

		if (m_SelectedObject)
		{
			GDebugRenderer.DrawAabb(m_SelectedObject->GetAabb());

			RVec3 pos = m_SelectedObject->GetPosition();
			GDebugRenderer.DrawLine(RVec3(pos.X() + 10000.0f, pos.Y(), pos.Z()), RVec3(pos.X() - 10000.0f, pos.Y(), pos.Z()));
			GDebugRenderer.DrawLine(RVec3(pos.X(), pos.Y() + 10000.0f, pos.Z()), RVec3(pos.X(), pos.Y() - 10000.0f, pos.Z()));
			GDebugRenderer.DrawLine(RVec3(pos.X(), pos.Y(), pos.Z() + 10000.0f), RVec3(pos.X(), pos.Y(), pos.Z() - 10000.0f));
		}

		// Update scene constant buffer
		SHADER_SCENE_BUFFER cbScene;

		cbScene.viewMatrix = viewMatrix;
		cbScene.projMatrix = projMatrix;
		cbScene.viewProjMatrix = viewProjMatrix;
		cbScene.cameraPos = m_CameraMatrix.GetRow(3);

		RConstantBuffers::cbScene.UpdateBufferData(&cbScene);

		// Update light constant buffer
		SHADER_LIGHT_BUFFER cbLight;
		ZeroMemory(&cbLight, sizeof(cbLight));

		// Setup ambient color
		cbLight.HighHemisphereAmbientColor = RVec4(1.0f, 1.0f, 1.0f, 1.0f);
		cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

		RConstantBuffers::cbLight.UpdateBufferData(&cbLight);

		RConstantBuffers::cbPerObject.BindBuffer();
		RConstantBuffers::cbScene.BindBuffer();
		RConstantBuffers::cbLight.BindBuffer();

		// Update screen buffer
		SHADER_GLOBAL_BUFFER cbScreen;
		ZeroMemory(&cbScreen, sizeof(cbScreen));

		cbScreen.UseGammaCorrection = GRenderer.UsingGammaCorrection();

		RConstantBuffers::cbGlobal.UpdateBufferData(&cbScreen);
		RConstantBuffers::cbGlobal.BindBuffer();


		GRenderer.D3DImmediateContext()->PSSetSamplers(0, 1, &m_SamplerState);
		GRenderer.D3DImmediateContext()->PSSetSamplers(2, 1, &m_SamplerComparisonState);
	}

	void EditorApp::RenderScene()
	{
		GRenderer.Clear();

		// Update object constant buffer
		SHADER_OBJECT_BUFFER cbObject;
		cbObject.worldMatrix = RMatrix4::IDENTITY;

		RConstantBuffers::cbPerObject.UpdateBufferData(&cbObject);

		m_Skybox.Draw();

		GRenderer.Clear(false);

		m_Scene.Render();

		cbObject.worldMatrix = RMatrix4::IDENTITY;
		RConstantBuffers::cbPerObject.UpdateBufferData(&cbObject);

		GDebugRenderer.Render();

		GRenderer.Clear(false);

		// Draw axises
		if (m_SelectedObject)
		{
			m_ColorShader->Bind();

			RVec3 cam_pos = m_CameraMatrix.GetTranslation();
			RVec3 obj_pos = m_SelectedObject->GetPosition();
			float dist = (cam_pos - obj_pos).Magnitude();
			dist = max(50.0f, min(100.0f, dist));
			RVec3 axis_pos = cam_pos + (m_SelectedObject->GetPosition() - cam_pos).GetNormalized() * dist;

			m_AxisMatrix = RMatrix4::CreateTranslation(axis_pos);
			cbObject.worldMatrix = m_AxisMatrix;

			RConstantBuffers::cbPerObject.UpdateBufferData(&cbObject);

			m_EditorAxis.Draw();
		}

		GRenderer.Present();
		GDebugRenderer.Reset();
	}

	void EditorApp::AddMeshObjectToScene(const char* path)
	{
		RSMeshObject* pObj = m_Scene.CreateMeshObject(path);
		RAabb aabb = pObj->GetAabb();
		RVec3 center = (aabb.pMin + aabb.pMax) * 0.5f;
		float radius = (aabb.pMax - center).Magnitude();

		RVec3 pos = m_CameraMatrix.GetTranslation() + m_CameraMatrix.GetForward() * radius - center;
		pObj->SetPosition(pos);
	}

	void EditorApp::LoadScene(const char* filename)
	{
		m_SelectedObject = nullptr;
		m_Scene.DestroyAllObjects();
		m_Scene.LoadFromFile(filename);
	}

	void EditorApp::SaveScene(const char* filename)
	{
		m_Scene.SaveToFile(filename);
	}

	RSceneObject* EditorApp::GetSelection()
	{
		return m_SelectedObject;
	}

	void EditorApp::SaveMeshMaterialFromSelection()
	{
		if (m_SelectedObject && m_SelectedObject->GetType() == SO_MeshObject)
		{
			((RSMeshObject*)m_SelectedObject)->SaveMaterialsToFile();
		}
	}

	void EditorApp::ExportAllAnimationsToBinaryFiles()
	{
		const vector<RMesh*>& meshVec = RResourceManager::Instance().GetMeshResources();
		for (UINT i = 0; i < meshVec.size(); i++)
		{
			RAnimation* anim = meshVec[i]->GetAnimation();
			if (anim)
			{
				string animFilename = meshVec[i]->GetPath();
				animFilename = RFileUtil::ReplaceExtension(animFilename, "ranim");
				//anim->SaveToFile(animFilename.c_str());
			}
		}
	}

	void EditorApp::RunScreenToCameraRayPicking(float x, float y)
	{
		RVec3 farPoint = RVec3(2.0f * x - 1.0f, -2.0f * y + 1.0f, 1.0f);
		RVec4 farPointVec4 = RVec4(farPoint) * m_InvViewProjMatrix;
		RVec3 farPointWorld = (farPointVec4 / farPointVec4.w).ToVec3();
		RVec3 camPos = m_CameraMatrix.GetTranslation();

		RRay ray(camPos, farPointWorld);
		RRay axis_ray = ray;

		if (m_SelectedObject)
		{
			axis_ray = ray.Transform(m_AxisMatrix.FastInverse());
		}

		m_MouseControlMode = MouseControlMode::None;

		if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis.GetAabb(AXIS_X)))
		{
			m_MouseControlMode = MouseControlMode::MoveX;
		}
		else if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis.GetAabb(AXIS_Y)))
		{
			m_MouseControlMode = MouseControlMode::MoveY;
		}
		else if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis.GetAabb(AXIS_Z)))
		{
			m_MouseControlMode = MouseControlMode::MoveZ;
		}
		else
		{
			struct RayPickingResult
			{
				float t;
				RSceneObject* obj;

				RayPickingResult(float _t, RSceneObject* _obj)
					: t(_t), obj(_obj)
				{}

				bool operator==(RSceneObject* _obj)
				{
					return obj == _obj;
				}
			};

			vector<RayPickingResult> rayPickingList;
			float tmin = FLT_MAX;

			for (vector<RSceneObject*>::iterator iter = m_Scene.GetSceneObjects().begin(); iter != m_Scene.GetSceneObjects().end(); iter++)
			{
				float t;
				if (ray.TestAabbIntersection((*iter)->GetAabb(), &t))
				{
					if ((*iter)->GetType() == SO_MeshObject)
					{
						RSMeshObject* meshObj = static_cast<RSMeshObject*>(*iter);
						for (int i = 0; i < meshObj->GetMeshElementCount(); i++)
						{
							if (ray.TestAabbIntersection(meshObj->GetMeshElementAabb(i).GetTransformedAabb(meshObj->GetTransformMatrix()), &t))
							{
								rayPickingList.push_back(RayPickingResult(t, *iter));
								break;
							}
						}
					}
					else
						rayPickingList.push_back(RayPickingResult(t, *iter));
				}
			}

			if (!rayPickingList.size())
			{
				m_SelectedObject = nullptr;
			}
			else
			{
				sort(rayPickingList.begin(), rayPickingList.end(), [](RayPickingResult a, RayPickingResult b){ return a.t < b.t; });
				vector<RayPickingResult>::iterator iter = find(rayPickingList.begin(), rayPickingList.end(), m_SelectedObject);
				if (iter != rayPickingList.end())
				{
					iter++;
					if (iter == rayPickingList.end())
						iter = rayPickingList.begin();
					m_SelectedObject = iter->obj;
				}
				else
				{
					m_SelectedObject = rayPickingList[0].obj;
				}
			}
		}

		if (m_MouseControlMode != MouseControlMode::None)
		{
			RInput.GetCursorPos(m_MouseDownX, m_MouseDownY);
		}
	}

	void EditorApp::DeleteSelection()
	{
		if (m_SelectedObject)
		{
			auto iter = std::find(m_Scene.GetSceneObjects().begin(), m_Scene.GetSceneObjects().end(), m_SelectedObject);
			m_Scene.DestroyObject(*iter);
			m_SelectedObject = nullptr;

			// FIXME: We should notify editor to update property grid or we'll crash the editor
		}
	}
#pragma managed
}