//=============================================================================
// EditorApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Stdafx.h"
#include "EditorApp.h"

namespace ManagedEngineWrapper
{
#pragma unmanaged
	EditorApp::EditorApp()
	{

	}

	EditorApp::~EditorApp()
	{
		m_Skybox.Release();
		m_Scene.Release();
	}

	bool EditorApp::Initialize()
	{
		GEngine.SetEditorMode(true);
		GEngine.SetUseCustomRenderingPipeline(false);

		m_Scene.Initialize();

		m_DefaultShader = RShaderManager::Instance().GetShaderResource("Default");

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

		m_CamYaw = m_CamPitch = 0.0f;
		m_SelectedObject = nullptr;
		m_MouseControlMode = MouseControlMode::None;

		CreateEditorObjects();

		GRenderer.SetActiveScene(&m_Scene);

		return true;
	}

	void EditorApp::UpdateScene(const RTimer& timer)
	{
		RVec3 moveVec(0.0f, 0.0f, 0.0f);

		RECT rwRect = GEngine.GetWindowRectInfo();
		int mx, my;
		RInput.GetCursorPosition(mx, my);

		if (mx >= rwRect.left && mx <= rwRect.right &&
			my >= rwRect.top && my <= rwRect.bottom)
		{
			if (RInput.GetBufferedKeyState(VK_RBUTTON) == EBufferedKeyState::Pressed)
			{
				RInput.HideCursor();
				RInput.LockCursor();
			}

			if (RInput.GetBufferedKeyState(VK_LBUTTON) == EBufferedKeyState::Pressed)
			{
				RECT rect = GEngine.GetWindowRectInfo();
				int cur_x, cur_y;

				RInput.GetCursorPosition(cur_x, cur_y);

				m_MouseDownX = cur_x;
				m_MouseDownY = cur_y;

				float fx = float(cur_x - rect.left) / float(rect.right - rect.left),
					  fy = float(cur_y - rect.top) / float(rect.bottom - rect.top);

				RunScreenToCameraRayPicking(fx, fy);
			}

			if (RInput.IsKeyDown(VK_RBUTTON))
			{
				int dx, dy;
				RInput.GetRelativeCursorPosition(dx, dy);
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
				if (RInput.IsKeyDown('Q'))
					moveVec -= RVec3(0.0f, 1.0f, 0.0f) * timer.DeltaTime() * camSpeed;
				if (RInput.IsKeyDown('E'))
					moveVec += RVec3(0.0f, 1.0f, 0.0f) * timer.DeltaTime() * camSpeed;
			}
		}

		if (RInput.GetBufferedKeyState(VK_RBUTTON) == EBufferedKeyState::Released)
		{
			RInput.ShowCursor();
			RInput.UnlockCursor();
		}

		if (RInput.GetBufferedKeyState(VK_LBUTTON) == EBufferedKeyState::Released)
		{
			m_MouseControlMode = MouseControlMode::None;
		}

		if (m_MouseControlMode != MouseControlMode::None)
		{
			if (m_SelectedObject)
			{
				int mdx, mdy;
				RInput.GetRelativeCursorPosition(mdx, mdy);

				RVec3 world_x = RVec3(1, 0, 0);
				RVec3 world_y = RVec3(0, 1, 0);
				RVec3 world_z = RVec3(0, 0, 1);

				RVec3 cam_right = m_EditorCamera->GetRightVector();
				RVec3 cam_up = m_EditorCamera->GetUpVector();

				float move_scale = GRenderer.GetClientHeight() / 500.0f;

				if (RInput.GetBufferedKeyState(VK_LBUTTON) == EBufferedKeyState::Pressed &&
					RInput.IsKeyDown(VK_LCONTROL))
				{
					RScene* Scene = m_SelectedObject->GetScene();
					m_SelectedObject = Scene->CloneObject(m_SelectedObject);
				}

				RVec3 pos = m_SelectedObject->GetPosition();

				if (RInput.IsKeyDown(VK_LMENU))
				{
					RQuat Rotation = m_SelectedObject->GetRotation();
					RQuat DeltaRotation;
					float Angle = SnapTo((float)mdx * 0.05f, PI/180.0f * 1.0f);

					switch (m_MouseControlMode)
					{
					case MouseControlMode::MoveX:
						DeltaRotation = RQuat::Euler(Angle, 0, 0);
						break;

					case MouseControlMode::MoveY:
						DeltaRotation = RQuat::Euler(0, Angle, 0);
						break;

					case MouseControlMode::MoveZ:
						DeltaRotation = RQuat::Euler(0, 0, Angle);
						break;
					}

					m_SelectedObject->SetRotation(Rotation * DeltaRotation);
				}
				else
				{
					switch (m_MouseControlMode)
					{
					case MouseControlMode::MoveX:
						{
							float x = pos.X() + (RVec3::Dot(world_x, cam_right) * mdx - RVec3::Dot(world_x, cam_up) * mdy) * move_scale;
							pos.SetX(SnapTo(x, 1.0f));
						}
						break;

					case MouseControlMode::MoveY:
						{
							float y = pos.Y() + (RVec3::Dot(world_y, cam_right) * mdx - RVec3::Dot(world_y, cam_up) * mdy) * move_scale;
							pos.SetY(SnapTo(y, 1.0f));
						}
						break;

					case MouseControlMode::MoveZ:
						{
							float z = pos.Z() + (RVec3::Dot(world_z, cam_right) * mdx - RVec3::Dot(world_z, cam_up) * mdy) * move_scale;
							pos.SetZ(SnapTo(z, 1.0f));
						}
						break;
					}
				}

				m_SelectedObject->SetPosition(pos);
			}
		}

		m_EditorCamera->SetRotation(RQuat::Euler(m_CamPitch, m_CamYaw, 0));
		m_EditorCamera->Translate(moveVec, ETransformSpace::Local);

		//RVec3 camPos = m_CameraMatrix.GetTranslation();
		//m_CameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);
		//m_CameraMatrix.SetTranslation(camPos + (RVec4(moveVec, 1.0f) * m_CameraMatrix).ToVec3());

		m_InvViewProjMatrix = m_EditorCamera->GetViewProjMatrix().Inverse();

		if (m_SelectedObject)
		{
			GDebugRenderer.DrawAabb(m_SelectedObject->GetAabb());

			RVec3 pos = m_SelectedObject->GetPosition();
			GDebugRenderer.DrawLine(RVec3(pos.X() + 10000.0f, pos.Y(), pos.Z()), RVec3(pos.X() - 10000.0f, pos.Y(), pos.Z()));
			GDebugRenderer.DrawLine(RVec3(pos.X(), pos.Y() + 10000.0f, pos.Z()), RVec3(pos.X(), pos.Y() - 10000.0f, pos.Z()));
			GDebugRenderer.DrawLine(RVec3(pos.X(), pos.Y(), pos.Z() + 10000.0f), RVec3(pos.X(), pos.Y(), pos.Z() - 10000.0f));
		}

		// Draw axises
		if (m_SelectedObject)
		{
			RVec3 cam_pos = m_EditorCamera->GetPosition();
			RVec3 obj_pos = m_SelectedObject->GetPosition();
			float dist = (cam_pos - obj_pos).Magnitude();
			dist = max(50.0f, min(100.0f, dist));
			RVec3 axis_pos = cam_pos + (m_SelectedObject->GetPosition() - cam_pos).GetNormalized() * dist;

			m_EditorAxis->SetPosition(axis_pos);
			m_EditorAxis->SetRotation(m_SelectedObject->GetRotation());
			m_AxisMatrix = m_EditorAxis->GetTransformMatrix();

			m_EditorAxis->SetVisible(true);
		}
		else
		{
			m_EditorAxis->SetVisible(false);
		}
	}

	void EditorApp::OnResize(int width, int height)
	{
		if (m_EditorCamera)
		{
			m_EditorCamera->SetAspectRatio((float)width / (float)height);
		}
	}

	RSceneObject* EditorApp::AddMeshObjectToScene(const char* MeshAssetPath)
	{
		RSMeshObject* pObj = m_Scene.CreateMeshObject(MeshAssetPath);
		RAabb aabb = pObj->GetAabb();
		RVec3 center = (aabb.pMin + aabb.pMax) * 0.5f;
		float radius = (aabb.pMax - center).Magnitude();

		RVec3 pos = m_EditorCamera->GetPosition() + m_EditorCamera->GetForwardVector() * radius - center;
		pos.SetX(SnapTo(pos.X(), 1.0f));
		pos.SetY(SnapTo(pos.Y(), 1.0f));
		pos.SetZ(SnapTo(pos.Z(), 1.0f));
		pObj->SetPosition(pos);

		string AssetName = RFileUtil::GetFileNameInPath(MeshAssetPath);
		AssetName = RFileUtil::StripExtension(AssetName);

		// Generate unique name in the scene
		const string ObjectName = m_Scene.GenerateUniqueObjectName(AssetName);
		pObj->SetName(ObjectName);

		return pObj;
	}

	void EditorApp::LoadScene(const char* filename)
	{
		m_SelectedObject = nullptr;
		m_Scene.DestroyAllObjects();
		m_Scene.LoadFromFile(filename);

		CreateEditorObjects();
	}

	void EditorApp::SaveScene(const char* filename)
	{
		m_Scene.SaveToFile(filename);
	}

	RSceneObject* EditorApp::GetSelection()
	{
		return m_SelectedObject;
	}

	void EditorApp::SetSelection(RSceneObject* SceneObject)
	{
		m_SelectedObject = SceneObject;
	}

	vector<RSceneObject*> EditorApp::GetSceneObjects() const
	{
		return m_Scene.EnumerateSceneObjects();
	}

	void EditorApp::SaveMeshMaterialFromSelection()
	{
		if (m_SelectedObject && m_SelectedObject->IsType<RSMeshObject>())
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
		RVec3 camPos = m_EditorCamera->GetPosition();

		RRay ray(camPos, farPointWorld);
		RRay axis_ray = ray;

		if (m_SelectedObject)
		{
			axis_ray = ray.Transform(m_AxisMatrix.FastInverse());
		}

		m_MouseControlMode = MouseControlMode::None;

		if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis->GetAabb(AXIS_X)))
		{
			m_MouseControlMode = MouseControlMode::MoveX;
		}
		else if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis->GetAabb(AXIS_Y)))
		{
			m_MouseControlMode = MouseControlMode::MoveY;
		}
		else if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis->GetAabb(AXIS_Z)))
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

			for (auto SceneObject : m_Scene.EnumerateSceneObjects())
			{
				float t;
				if (ray.TestAabbIntersection(SceneObject->GetAabb(), &t))
				{
					if (SceneObject->IsType<RSMeshObject>())
					{
						RSMeshObject* meshObj = static_cast<RSMeshObject*>(SceneObject);
						for (int i = 0; i < meshObj->GetMeshElementCount(); i++)
						{
							if (ray.TestAabbIntersection(meshObj->GetMeshElementAabb(i).GetTransformedAabb(meshObj->GetTransformMatrix()), &t))
							{
								rayPickingList.push_back(RayPickingResult(t, SceneObject));
								break;
							}
						}
					}
					else
					{
						rayPickingList.push_back(RayPickingResult(t, SceneObject));
					}
				}
			}

			if (!rayPickingList.size())
			{
				m_SelectedObject = nullptr;
			}
			else
			{
				sort(rayPickingList.begin(), rayPickingList.end(), [](RayPickingResult a, RayPickingResult b){ return a.t < b.t; });
				auto iter = find(rayPickingList.begin(), rayPickingList.end(), m_SelectedObject);
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
	}

	bool EditorApp::DeleteSelection()
	{
		if (m_SelectedObject)
		{
			vector<RSceneObject*> SceneObjects = m_Scene.EnumerateSceneObjects();
			auto iter = std::find(SceneObjects.begin(), SceneObjects.end(), m_SelectedObject);
			m_Scene.DestroyObject(*iter);
			m_SelectedObject = nullptr;

			// FIXME: We should notify editor to update property grid or we'll crash the editor

			return true;
		}

		return false;
	}

	float EditorApp::SnapTo(float Value, float Unit)
	{
		return Unit * int((Value + Unit * 0.5f) / Unit);
	}

	void EditorApp::CreateEditorObjects()
	{
		m_EditorAxis = m_Scene.CreateSceneObjectOfType<EditorAxis>("EditorAxis");
		m_EditorAxis->SetVisible(false);

		// Create editor camera
		m_EditorCamera = m_Scene.CreateSceneObjectOfType<RCamera>("EditorCamera", CF_InternalObject | CF_NoSerialization);
		m_EditorCamera->SetupView(65.0f, GRenderer.AspectRatio(), 1.0f, 10000.0f);

		RSceneObject* GlobalLightInfo = m_Scene.CreateSceneObjectOfType<RSceneObject>("DirectionalLight", CF_NoSerialization);
		RDirectionalLightComponent* DirLightComponent = GlobalLightInfo->AddNewComponent<RDirectionalLightComponent>();
		DirLightComponent->SetParameters({ RVec3(-0.5f, 1, -0.3f), RColor(0.5f, 0.5f, 0.5f) });
	}

#pragma managed
}