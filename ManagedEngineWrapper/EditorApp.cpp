//=============================================================================
// EditorApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Stdafx.h"
#include "EditorApp.h"

namespace ManagedEngineWrapper
{
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
		RResourceManager::Instance().LoadResource<RMesh>("/Sphere.fbx");
		RResourceManager::Instance().LoadResource<RMesh>("/SpeedballPlayer.fbx");
		RResourceManager::Instance().LoadResource<RMesh>("/AO_Scene.fbx");
		RResourceManager::Instance().LoadResource<RMesh>("/tachikoma.fbx");
		RResourceManager::Instance().LoadResource<RMesh>("/Island.fbx");
		RResourceManager::Instance().LoadResource<RMesh>("/city.fbx");
#endif

		m_Skybox.CreateSkybox("/powderpeak.dds");

		m_CamYaw = m_CamPitch = 0.0f;
		m_SelectedObject = nullptr;
		m_MouseControlMode = MouseControlMode::None;

		CreateEditorObjects();

		GRenderer.SetActiveScene(&m_Scene);

		return true;
	}

	void EditorApp::UpdateScene(const RTimer& timer)
	{
		if (m_bInputEnabled)
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
					RVec2 CursorPoint = GetCursorPointInViewport();
					RunScreenToCameraRayPicking(CursorPoint);
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

					// Hold ctrl key to clone objects
					if (RInput.GetBufferedKeyState(VK_LBUTTON) == EBufferedKeyState::Pressed &&
						RInput.IsKeyDown(VK_LCONTROL))
					{
						RScene* Scene = m_SelectedObject->GetScene();
						m_SelectedObject = Scene->CloneObject(m_SelectedObject);
					}

					RVec3 pos = m_SelectedObject->GetPosition();

					// Hold alt key to rotate objects
					if (RInput.IsKeyDown(VK_LMENU))
					{
						RQuat Rotation = m_SelectedObject->GetRotation();
						RQuat DeltaRotation;
						float Angle = SnapTo((float)mdx * 0.05f, PI / 180.0f * 1.0f);

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
						// Move scene object along selected axis

						// TODO: Support local space later
						RVec3 AxisVector = RVec3(0, 0, 0);

						switch (m_MouseControlMode)
						{
						case MouseControlMode::MoveX:
							AxisVector = RVec3(1, 0, 0);
							break;

						case MouseControlMode::MoveY:
							AxisVector = RVec3(0, 1, 0);
							break;

						case MouseControlMode::MoveZ:
							AxisVector = RVec3(0, 0, 1);
							break;

						default:
							break;
						}

						if (AxisVector.SquaredMagitude() > FLT_EPSILON)
						{
							// Get the moving referencing plane and calculate a intersection point with the camera ray.
							// The moving offset is the projected distance of intersection point and start moving on the moving axis.
							RPlane Plane = GetAxisPlane(m_CursorStartPosition, AxisVector);
							RVec2 CursorPoint = GetCursorPointInViewport();
							RRay CameraRay = MakeRayFromViewportPoint(CursorPoint);
							float Dist;

							if (CameraRay.TestIntersectionWithPlane(Plane, &Dist))
							{
								RVec3 HitPoint = CameraRay.Origin + CameraRay.Direction * Dist;

								float Offset = SnapTo(RVec3::Dot(HitPoint - m_CursorStartPosition, AxisVector), 1.0f);
								pos = m_ObjectStartPosition + AxisVector * Offset;
							}
						}
					}

					m_SelectedObject->SetPosition(pos);
				}
			}

			m_EditorCamera->SetRotation(RQuat::Euler(m_CamPitch, m_CamYaw, 0));
			m_EditorCamera->Translate(moveVec, ETransformSpace::Local);
		}

		DrawGrid();

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
			//m_EditorAxis->SetRotation(m_SelectedObject->GetRotation());
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

		// Offset from center of the bound to object's origin
		RVec3 Offset = pObj->GetWorldPosition() - center;

		// Fit the object's aabb into vertical FOV of the camera
		float HalfFov = m_EditorCamera->GetFOV() * 0.5f;
		float Distance = radius / tanf(RMath::DegreeToRadian(HalfFov * 0.8f));

		// Place the object in front of the camera, at a good distance
		RVec3 pos = m_EditorCamera->GetPosition() + m_EditorCamera->GetForwardVector() * Distance + Offset;
		pos.SetX(SnapTo(pos.X(), 1.0f));
		pos.SetY(SnapTo(pos.Y(), 1.0f));
		pos.SetZ(SnapTo(pos.Z(), 1.0f));
		pObj->SetPosition(pos);

		string AssetName = RFileUtil::GetFileNameInPath(MeshAssetPath);
		AssetName = RFileUtil::StripExtension(AssetName);

		// Generate a unique name for the object
		const string ObjectName = m_Scene.GenerateUniqueObjectName(AssetName);
		pObj->SetName(ObjectName);

		return pObj;
	}

	void EditorApp::LoadScene(const char* filename)
	{
		m_SelectedObject = nullptr;
		m_Scene.DestroyAllObjects();

		// Check if file name is under base assets path by comparing their full path
		const string FullPath = RFileUtil::GetFullPath(filename);
		const string BaseAssetPath = RFileUtil::GetFullPath(RResourceManager::GetAssetsBasePath());
		if (FullPath.find(BaseAssetPath) == 0)
		{
			string MapAssetPath = string("/") + FullPath.substr(BaseAssetPath.size());
			m_Scene.LoadFromFile(MapAssetPath);
		}
		else
		{
			string ErrorMsg = string("Unable to open map file:\n");
			ErrorMsg += FullPath;
			ErrorMsg += string("\n\nPlease select a map from current base assets path: ");
			ErrorMsg += BaseAssetPath;

			MessageBoxA(NULL, ErrorMsg.c_str(), "Load Map Error", MB_ICONWARNING);
		}

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
		vector<RMesh*> meshVec = RResourceManager::Instance().GetMeshResources();
		for (UINT i = 0; i < meshVec.size(); i++)
		{
			RAnimation* anim = meshVec[i]->GetAnimation();
			if (anim)
			{
				string animFilename = meshVec[i]->GetFileSystemPath();
				animFilename = RFileUtil::ReplaceExtension(animFilename, "ranim");
				//anim->SaveToFile(animFilename.c_str());
			}
		}
	}

	void EditorApp::SetInputEnabled(bool bEnabled)
	{
		m_bInputEnabled = bEnabled;
	}

	void EditorApp::RunScreenToCameraRayPicking(const RVec2& Point)
	{
		RRay CameraRay = MakeRayFromViewportPoint(Point);
		m_MouseControlMode = MouseControlMode::None;

		if (m_SelectedObject)
		{
			// Transform camera ray to axis local space
			RRay axis_ray = CameraRay.Transform(m_AxisMatrix.FastInverse());

			if (axis_ray.TestIntersectionWithAabb(m_EditorAxis->GetLocalAabb(AXIS_X)))
			{
				m_MouseControlMode = MouseControlMode::MoveX;
			}
			else if (axis_ray.TestIntersectionWithAabb(m_EditorAxis->GetLocalAabb(AXIS_Y)))
			{
				m_MouseControlMode = MouseControlMode::MoveY;
			}
			else if (axis_ray.TestIntersectionWithAabb(m_EditorAxis->GetLocalAabb(AXIS_Z)))
			{
				m_MouseControlMode = MouseControlMode::MoveZ;
			}

			if (m_MouseControlMode != MouseControlMode::None)
			{
				// Since the axis may move along camera direction for scaling, the hit point is not always what we're looking for.
				// We'll check ray intersection with axis planes for the actual start position.

				RVec3 ObjectPosition = m_SelectedObject->GetWorldPosition();
				RPlane AxisPlane;

				switch (m_MouseControlMode)
				{
				case MouseControlMode::MoveX:
					AxisPlane = GetAxisPlane(ObjectPosition, RVec3(1, 0, 0));
					break;

				case MouseControlMode::MoveY:
					AxisPlane = GetAxisPlane(ObjectPosition, RVec3(0, 1, 0));
					break;

				case MouseControlMode::MoveZ:
					AxisPlane = GetAxisPlane(ObjectPosition, RVec3(0, 0, 1));
					break;

				default:
					break;
				}

				if (AxisPlane.IsValid())
				{
					float Dist;
					if (CameraRay.TestIntersectionWithPlane(AxisPlane, &Dist))
					{
						m_CursorStartPosition = CameraRay.GetPointAtDistance(Dist);
					}
				}

				m_ObjectStartPosition = ObjectPosition;
			}
		}

		if (m_MouseControlMode == MouseControlMode::None)
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
				if (CameraRay.TestIntersectionWithAabb(SceneObject->GetAabb(), &t))
				{
					if (SceneObject->IsType<RSMeshObject>())
					{
						RSMeshObject* meshObj = static_cast<RSMeshObject*>(SceneObject);
						for (int i = 0; i < meshObj->GetMeshElementCount(); i++)
						{
							if (CameraRay.TestIntersectionWithAabb(meshObj->GetMeshElementAabb(i).GetTransformedAabb(meshObj->GetTransformMatrix()), &t))
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

	void EditorApp::SetOnAsyncResourceLoadedCallback(NativeAsyncResourceLoadedCallback Func)
	{
		auto AsyncResourceLoadedCallback = Func;
		RResourceManager::Instance().OnResourceFinishedAsyncLoading.BindLambda([AsyncResourceLoadedCallback](RResourceBase* Resource)
		{
			if (AsyncResourceLoadedCallback != nullptr)
			{
				const char* ResourcePath = Resource->GetAssetPath().c_str();
				(*AsyncResourceLoadedCallback)(ResourcePath);
			}
		});
	}

	float EditorApp::SnapTo(float Value, float Unit)
	{
		return Unit * int((Value + Unit * 0.5f) / Unit);
	}

	RRay EditorApp::MakeRayFromViewportPoint(const RVec2& Point)
	{
		assert(m_EditorCamera != nullptr);

		RVec4 FarPoint = RVec4(2.0f * Point.x - 1.0f, -2.0f * Point.y + 1.0f, 1.0f, 1.0f);
		FarPoint = FarPoint * m_EditorCamera->GetViewProjMatrix().Inverse();
		RVec3 FarPointWorld = FarPoint.ToVec3() / FarPoint.w;
		RVec3 CameraPosition = m_EditorCamera->GetPosition();

		return RRay(CameraPosition, FarPointWorld);
	}

	RVec2 EditorApp::GetCursorPointInViewport() const
	{
		RECT WindowRect = GEngine.GetWindowRectInfo();

		int CurX, CurY;
		RInput.GetCursorPosition(CurX, CurY);

		// Mouse cursor relative position in viewport
		return RVec2(
			float(CurX - WindowRect.left) / float(WindowRect.right - WindowRect.left),
			float(CurY - WindowRect.top) / float(WindowRect.bottom - WindowRect.top));
	}

	RPlane EditorApp::GetAxisPlane(const RVec3& Point, const RVec3& AxisDirection) const
	{
		RVec3 p0 = Point;
		RVec3 p1 = Point + AxisDirection;
		RVec3 SideVec = RVec3::Cross((m_EditorCamera->GetPosition() - Point).GetNormalized(), AxisDirection);
		RVec3 p2 = Point + SideVec;

		return RPlane(p0, p1, p2);
	}

	void EditorApp::DrawGrid() const
	{
		static const int Size = 1000;
		static const int Step = 50;
		static const int BoldStep = 100;
		static const RColor BoldColor = RColor(0.5f, 0.5f, 0.65f);
		static const RColor LineColor = RColor(0.25f, 0.25f, 0.5f);

		for (int t = -Size; t <= Size; t += Step)
		{
			RVec3 Start((float)t, 0, -(float)Size);
			RVec3 End((float)t, 0, (float)Size);
			GDebugRenderer.DrawLine(Start, End, (t % BoldStep == 0) ? BoldColor : LineColor);
		}

		for (int t = -Size; t <= Size; t += Step)
		{
			RVec3 Start((float)Size, 0, (float)t);
			RVec3 End(-(float)Size, 0, (float)t);
			GDebugRenderer.DrawLine(Start, End, (t % BoldStep == 0) ? BoldColor : LineColor);
		}
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
}
