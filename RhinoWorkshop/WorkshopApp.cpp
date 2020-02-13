//=============================================================================
// WorkshopApp.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "WorkshopApp.h"

#include "../RhinoGameUtils/RFreeFlyCameraControl.h"

WorkshopApp::WorkshopApp()
	: bShowOpenDialog(false)
	, bShowAssetEditor(true)
	, SelectedObject(nullptr)
{

}

bool WorkshopApp::Initialize()
{
	GEngine.SetEditorMode(true);

	RResourceManager::Instance().LoadAllResources(EResourceLoadMode::Immediate);
	ResourcePreviewBuilder.BuildPreviewForAllResources();

	GSceneManager.DefaultScene()->Initialize();

	// Create default camera and light on editor start
	PostMapLoaded();

	return true;
}

void WorkshopApp::UpdateScene(const RTimer& timer)
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2((float)WindowWidth, 0));

	ImGuiWindowFlags Flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		//ImGuiWindowFlags_NoBackground |
		//ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_MenuBar;

	// Disable corner rounding on the main menu
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::Begin("Rhino Workshop", nullptr, Flags);
	ImGui::PopStyleVar();

	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Map"))
				{
					RScene* DefaultScene = GSceneManager.DefaultScene();
					assert(DefaultScene);

					SetSelectedObject(nullptr);
					DefaultScene->DestroyAllObjects();

					PostMapLoaded();
				}

				// Open menu
				if (ImGui::MenuItem("Open"))
				{
					// Note: Since the menu code path won't execute when the popup is open, we need to indicate the state by a variable
					bShowOpenDialog = true;
					UpdateEngineMapList();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Save Map"))
				{
					RScene* DefaultScene = GSceneManager.DefaultScene();
					assert(DefaultScene);

					DefaultScene->SaveToFile((RResourceManager::GetAssetsBasePath() + CurrentMapPath).c_str());
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit"))
				{
					PostQuitMessage(0);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Mesh Object"))
				{
					RScene* DefaultScene = GSceneManager.DefaultScene();
					RSMeshObject* NewObject = DefaultScene->CreateSceneObjectOfType<RSMeshObject>("New Object");

					RVec3 CreatePosition = DefaultScene->GetRenderCamera() ? DefaultScene->GetRenderCamera()->GetPosition() : RVec3::Zero();
					NewObject->SetPosition(CreatePosition);

					RMesh* SelectedMesh = AssetsViewWindow.GetSelectedResource() ? AssetsViewWindow.GetSelectedResource()->CastTo<RMesh>() : nullptr;
					if (SelectedMesh)
					{
						NewObject->SetMesh(SelectedMesh);
					}

					SetSelectedObject(NewObject);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Show Assets View", nullptr, &AssetsViewWindow.bShowWindow);
				ImGui::MenuItem("Show Asset Editor", nullptr, &AssetEditorWindow.bShowWindow);
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		float CameraSpeed = GetCameraSpeed();
		ImGui::SetNextItemWidth(80);
		if (ImGui::DragFloat("Camera Speed", &CameraSpeed, 10.0f, 1.0f, FLT_MAX, "%.1f"))
		{
			SetCameraSpeed(CameraSpeed);
		}

		// The tree view window of the scene
		DisplaySceneViewWindow();

		// The asset explorer window
		AssetsViewWindow.ShowWindow(ResourcePreviewBuilder, AssetEditorWindow.bShowWindow);

		AssetEditorWindow.SetEditingResource(AssetsViewWindow.GetEditingResource());
		AssetEditorWindow.ShowWindow(ResourcePreviewBuilder, AssetsViewWindow.GetSelectedResource());

		if (SelectedObject)
		{
			ImGui::Begin("Object View", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::Text("Class: %s", SelectedObject->GetClassName());

			char ObjectName[256];
			strcpy_s(ObjectName, SelectedObject->GetName().c_str());
			if (ImGui::InputText("Name", ObjectName, sizeof(ObjectName)))
			{
				SelectedObject->SetName(ObjectName);
			}

			static bool TransformTreeOpen = true;
			ImGui::SetNextTreeNodeOpen(TransformTreeOpen);
			if (TransformTreeOpen = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_None))
			{
				if (ImGui::DragFloat3("Position", PosValueArray))
				{
					SelectedObject->SetPosition(RVec3(PosValueArray));
				}

				// TODO: Quaternion to Euler is NOT stable. Values my change when clicking on the same object a second time
				if (ImGui::DragFloat3("Rotation", RotValueArray))
				{
					for (int i = 0; i < 3; i++)
					{
						RotValueArray[i] = RMath::UnwindDegree(RotValueArray[i]);
					}

					SelectedObject->SetRotation(RQuat::Euler(RVec3(
						RMath::DegreeToRadian(RotValueArray[0]),
						RMath::DegreeToRadian(RotValueArray[1]),
						RMath::DegreeToRadian(RotValueArray[2])
					)));
				}

				if (ImGui::DragFloat3("Scale", ScaleValueArray))
				{
					SelectedObject->SetScale(RVec3(ScaleValueArray));
				}
			}

			if (RSMeshObject* MeshObject = SelectedObject->CastTo<RSMeshObject>())
			{
				static bool MeshTreeOpen = true;
				ImGui::SetNextTreeNodeOpen(MeshTreeOpen);
				if (MeshTreeOpen = ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_None))
				{
					// Button for mesh selection
					ImGui::Button(".."); ImGui::SameLine();
					if (ImGui::Button("->"))
					{
						if (auto Resource = AssetsViewWindow.GetSelectedResource())
						{
							if (RMesh* MeshAsset = Resource->CastTo<RMesh>())
							{
								MeshObject->SetMesh(MeshAsset);
							}
						}
					}
					ImGui::SameLine();

					RMesh* Mesh = MeshObject->GetMesh();
					std::string AssetPath;
					if (Mesh)
					{
						AssetPath = Mesh->GetAssetPath();
					}

					char MeshAssetName[256];
					strcpy_s(MeshAssetName, AssetPath.c_str());
					ImGui::InputText("Mesh", MeshAssetName, sizeof(MeshAssetName));
				}

				static bool MaterialTreeOpen = true;
				ImGui::SetNextTreeNodeOpen(MaterialTreeOpen);
				if (MaterialTreeOpen = ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_None))
				{
					//const auto& Materials = MeshObject->GetMaterials();
					RMesh* Mesh = MeshObject->GetMesh();

					if (Mesh)
					{
						for (int i = 0; i < Mesh->GetMeshElementCount(); i++)
						{
							std::string AssignButtonText("->##" + std::to_string(i));
							if (ImGui::Button(AssignButtonText.c_str()))
							{
								if (auto Resource = AssetsViewWindow.GetSelectedResource())
								{
									if (RMaterial* MaterialAsset = Resource->CastTo<RMaterial>())
									{
										MeshObject->SetMaterialSlot(i, MaterialAsset);
									}
								}
							}

							ImGui::SameLine();

							std::string MaterialAssetPath;
							std::string Label = "Slot " + std::to_string(i) + ": " + Mesh->GetMeshElements()[i].GetName();

							if (i < MeshObject->GetNumMaterials())
							{
								RMaterial* Material = MeshObject->GetMaterial(i);
								MaterialAssetPath = Material ? Material->GetAssetPath() : "";
							}

							char MaterialAssetName[256];
							strcpy_s(MaterialAssetName, MaterialAssetPath.c_str());
							if (ImGui::InputText(Label.c_str(), MaterialAssetName, sizeof(MaterialAssetName)))
							{

							}
						}
					}
				}
			}

			ImGui::End();
		}

		if (bShowOpenDialog)
		{
			ImGui::OpenPopup("Open Map");
		}

		if (ImGui::BeginPopupModal("Open Map", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (EngineMaps.size() > 0)
			{
				std::vector<const char*> Items;
				Items.resize(EngineMaps.size());
				for (int i = 0; i < EngineMaps.size(); i++)
				{
					Items[i] = EngineMaps[i].c_str();
				}

				static int SelectIdx = 0;

				ImGui::Text("Select a map:");
				ImGui::ListBox("", &SelectIdx, Items.data(), (int)Items.size(), (int)Items.size());

				if (ImGui::Button("Open"))
				{
					CurrentMapPath = EngineMaps[SelectIdx];
					RScene* DefaultScene = GSceneManager.DefaultScene();
					assert(DefaultScene);

					SetSelectedObject(nullptr);
					DefaultScene->DestroyAllObjects();
					DefaultScene->LoadFromFile(CurrentMapPath);

					PostMapLoaded();

					ImGui::CloseCurrentPopup();
					bShowOpenDialog = false;
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					bShowOpenDialog = false;
				}
			}
			else
			{
				ImGui::Text("No map found in Assets folder!");
				if (ImGui::Button("Close"))
				{
					ImGui::CloseCurrentPopup();
					bShowOpenDialog = false;
				}
			}

			ImGui::EndPopup();
		}
	}
	ImGui::End();

	if (RInput.GetBufferedKeyState(VK_LBUTTON) == EBufferedKeyState::Pressed)
	{
		RVec2 CursorPoint = GetMousePositionInViewport();
		SelectSceneObjectAtCursor(CursorPoint);
		if (SelectedObject)
		{
			RVec3 Position = SelectedObject->GetPosition();
			PosValueArray[0] = Position.X();
			PosValueArray[1] = Position.Y();
			PosValueArray[2] = Position.Z();

			RVec3 Rotation = SelectedObject->GetRotation().ToEuler();
			RotValueArray[0] = RMath::RadianToDegree(Rotation.X());
			RotValueArray[1] = RMath::RadianToDegree(Rotation.Y());
			RotValueArray[2] = RMath::RadianToDegree(Rotation.Z());

			RVec3 Scale = SelectedObject->GetScale();
			ScaleValueArray[0] = Scale.X();
			ScaleValueArray[1] = Scale.Y();
			ScaleValueArray[2] = Scale.Z();
		}
	}

	if (RInput.GetBufferedKeyState('F') == EBufferedKeyState::KeyDown)
	{
		FocusOnSelection();
	}

	if (SelectedObject)
	{
		GDebugRenderer.DrawAabb(SelectedObject->GetAabb());
	}
}

void WorkshopApp::RenderScene()
{
	GRenderer.Clear();
}

void WorkshopApp::OnResize(int width, int height)
{
	WindowWidth = width;
	WindowHeight = height;

	RScene* DefaultScene = GSceneManager.DefaultScene();
	for (auto& Camera : DefaultScene->FindAllObjectsOfType<RCamera>())
	{
		Camera->SetAspectRatio((float)width / (float)height);
	}
}

void WorkshopApp::UpdateEngineMapList()
{
	EngineMaps = RFileUtil::GetFilesInDirectoryAndSubdirectories(RResourceManager::GetAssetsBasePath(), "*.rmap");
}

void WorkshopApp::PostMapLoaded()
{
	// Add default camera to scene
	RScene* DefaultScene = GSceneManager.DefaultScene();
	RCamera* Camera = DefaultScene->CreateSceneObjectOfType<RCamera>("Editor Camera", CF_NoSerialization);
	RFreeFlyCameraControl* CameraControl = Camera->AddNewComponent<RFreeFlyCameraControl>();
	CameraControl->SetEnabled(true);

	// Create default lighting
	RSceneObject* GlobalLightInfo = DefaultScene->CreateSceneObjectOfType<RSceneObject>("DirectionalLight", CF_NoSerialization);
	RDirectionalLightComponent* DirLightComponent = GlobalLightInfo->AddNewComponent<RDirectionalLightComponent>();
	DirLightComponent->SetParameters({ RVec3(sinf(1.0f) * 0.5f, 0.25f, cosf(1.0) * 0.5f), RColor(1.0f, 1.0f, 0.8f, 1.0f) });
}

RFreeFlyCameraControl* WorkshopApp::GetFreeFlyCamera() const
{
	RScene* DefaultScene = GSceneManager.DefaultScene();
	if (DefaultScene)
	{
		RCamera* Camera = DefaultScene->GetRenderCamera();
		if (Camera)
		{
			return Camera->FindComponent<RFreeFlyCameraControl>();
		}
	}

	return nullptr;
}

float WorkshopApp::GetCameraSpeed() const
{
	RFreeFlyCameraControl* EditorCamera = GetFreeFlyCamera();
	if (EditorCamera)
	{
		return EditorCamera->GetNavigationSpeed();
	}

	return -1;
}

void WorkshopApp::SetCameraSpeed(float InSpeed)
{
	RFreeFlyCameraControl* EditorCamera = GetFreeFlyCamera();
	if (EditorCamera)
	{
		EditorCamera->SetNavigationSpeed(InSpeed);
	}
}

void WorkshopApp::FocusOnSelection()
{
	RFreeFlyCameraControl* EditorCamera = GetFreeFlyCamera();
	if (EditorCamera && SelectedObject)
	{
		EditorCamera->FocusOnObject(SelectedObject);
	}
}

void WorkshopApp::SetSelectedObject(RSceneObject* InSelected)
{
	SelectedObject = InSelected;
	if (SelectedObject)
	{
		RVec3 Position = SelectedObject->GetPosition();
		PosValueArray[0] = Position.X();
		PosValueArray[1] = Position.Y();
		PosValueArray[2] = Position.Z();

		RVec3 Rotation = SelectedObject->GetRotation().ToEuler();
		RotValueArray[0] = RMath::RadianToDegree(Rotation.X());
		RotValueArray[1] = RMath::RadianToDegree(Rotation.Y());
		RotValueArray[2] = RMath::RadianToDegree(Rotation.Z());

		RVec3 Scale = SelectedObject->GetScale();
		ScaleValueArray[0] = Scale.X();
		ScaleValueArray[1] = Scale.Y();
		ScaleValueArray[2] = Scale.Z();
	}
}

std::vector<RSceneObject*> WorkshopApp::GetAllSceneObjects() const
{
	RScene* DefaultScene = GSceneManager.DefaultScene();
	if (DefaultScene)
	{
		return DefaultScene->EnumerateSceneObjects();
	}

	return std::vector<RSceneObject*>();
}

RVec2 WorkshopApp::GetMousePositionInViewport() const
{
	RECT ClientRect = GEngine.GetClientRectInfo();

	int CurX, CurY;
	RInput.GetCursorClientPosition(CurX, CurY);

	// Mouse cursor relative position in viewport
	return RVec2(
		float(CurX - ClientRect.left) / float(ClientRect.right - ClientRect.left),
		float(CurY - ClientRect.top) / float(ClientRect.bottom - ClientRect.top));
}

RRay WorkshopApp::MakeRayFromViewportPoint(const RVec2& Point) const
{
	auto Cameras = GSceneManager.DefaultScene()->FindAllObjectsOfType<RCamera>();

	if (Cameras.size() > 0)
	{
		RVec4 FarPoint = RVec4(2.0f * Point.x - 1.0f, -2.0f * Point.y + 1.0f, 1.0f, 1.0f);
		FarPoint = FarPoint * Cameras[0]->GetViewProjMatrix().Inverse();
		RVec3 FarPointWorld = FarPoint.ToVec3() / FarPoint.w;
		RVec3 CameraPosition = Cameras[0]->GetPosition();

		return RRay(CameraPosition, FarPointWorld);
	}
	
	return RRay();
}

void WorkshopApp::SelectSceneObjectAtCursor(const RVec2& Point)
{
	RRay CameraRay = MakeRayFromViewportPoint(Point);

	//if (m_SelectedObject)
	//{
	//	// Transform camera ray to axis local space
	//	RRay axis_ray = CameraRay.Transform(m_AxisMatrix.FastInverse());

	//	if (axis_ray.TestIntersectionWithAabb(m_EditorAxis->GetLocalAabb(AXIS_X)))
	//	{
	//		m_MouseControlMode = MouseControlMode::MoveX;
	//	}
	//	else if (axis_ray.TestIntersectionWithAabb(m_EditorAxis->GetLocalAabb(AXIS_Y)))
	//	{
	//		m_MouseControlMode = MouseControlMode::MoveY;
	//	}
	//	else if (axis_ray.TestIntersectionWithAabb(m_EditorAxis->GetLocalAabb(AXIS_Z)))
	//	{
	//		m_MouseControlMode = MouseControlMode::MoveZ;
	//	}

	//	if (m_MouseControlMode != MouseControlMode::None)
	//	{
	//		// Since the axis may move along camera direction for scaling, the hit point is not always what we're looking for.
	//		// We'll check ray intersection with axis planes for the actual start position.

	//		RVec3 ObjectPosition = m_SelectedObject->GetWorldPosition();
	//		RPlane AxisPlane;

	//		switch (m_MouseControlMode)
	//		{
	//		case MouseControlMode::MoveX:
	//			AxisPlane = GetAxisPlane(ObjectPosition, RVec3(1, 0, 0));
	//			break;

	//		case MouseControlMode::MoveY:
	//			AxisPlane = GetAxisPlane(ObjectPosition, RVec3(0, 1, 0));
	//			break;

	//		case MouseControlMode::MoveZ:
	//			AxisPlane = GetAxisPlane(ObjectPosition, RVec3(0, 0, 1));
	//			break;

	//		default:
	//			break;
	//		}

	//		if (AxisPlane.IsValid())
	//		{
	//			float Dist;
	//			if (CameraRay.TestIntersectionWithPlane(AxisPlane, &Dist))
	//			{
	//				m_CursorStartPosition = CameraRay.GetPointAtDistance(Dist);
	//			}
	//		}

	//		m_ObjectStartPosition = ObjectPosition;
	//	}
	//}

	//if (m_MouseControlMode == MouseControlMode::None)
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

		std::vector<RayPickingResult> rayPickingList;
		float tmin = FLT_MAX;

		for (auto SceneObject : GSceneManager.DefaultScene()->EnumerateSceneObjects())
		{
			float t;
			if (CameraRay.TestIntersectionWithAabb(SceneObject->GetAabb(), &t))
			{
				if (SceneObject->CanCastTo<RSMeshObject>())
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
			//m_SelectedObject = nullptr;
		}
		else
		{
			sort(rayPickingList.begin(), rayPickingList.end(), [](RayPickingResult a, RayPickingResult b) { return a.t < b.t; });
			//auto iter = find(rayPickingList.begin(), rayPickingList.end(), m_SelectedObject);
			//if (iter != rayPickingList.end())
			//{
			//	iter++;
			//	if (iter == rayPickingList.end())
			//		iter = rayPickingList.begin();
			//	m_SelectedObject = iter->obj;
			//}
			//else
			{
				SetSelectedObject(rayPickingList[0].obj);
			}
		}
	}
}

void WorkshopApp::DisplaySceneViewWindow()
{
	if (ImGui::Begin("Scene View", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		auto SceneObjects = GetAllSceneObjects();

		if (SceneObjects.size() > 0)
		{
			for (int i = 0; i < (int)SceneObjects.size(); i++)
			{
				bool bIsSelected = (SelectedObject == SceneObjects[i]);
				std::string ObjectName = SceneObjects[i]->GetName();
				if (ObjectName == "")
				{
					ObjectName = "(no name)";
				}

				// Append unique ids to the label to avoid conflicts
				ObjectName += "##" + std::to_string(i);

				if (ImGui::Selectable(ObjectName.c_str(), &bIsSelected))
				{
					SetSelectedObject(SceneObjects[i]);
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					FocusOnSelection();
				}
			}
		}
		else
		{
			ImGui::Text("(No objects)");
		}
	}
	ImGui::End();
}
