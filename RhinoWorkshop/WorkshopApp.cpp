//=============================================================================
// WorkshopApp.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "WorkshopApp.h"

#include "../RhinoGameUtils/RFreeFlyCameraControl.h"
#include "REditorAxis.h"
#include "EditorCommon.h"

WorkshopApp::WorkshopApp()
	: bShowOpenDialog(false)
	, bShowAssetEditor(true)
	, SelectedObject(nullptr)
	, EditorAxis(nullptr)
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

			bool bNoCulling = SelectedObject->IsNoCulling();
			if (ImGui::Checkbox("No Culling", &bNoCulling))
			{
				SelectedObject->SetNoCulling(bNoCulling);
			}

			bool bNoShadow = SelectedObject->IsNoShadow();
			if (ImGui::Checkbox("No Shadow", &bNoShadow))
			{
				SelectedObject->SetNoShadow(bNoShadow);
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

	bool bUpdateTransformInWidgets = false;

	if (RInput.GetBufferedKeyState(VK_LBUTTON) == EBufferedKeyState::Pressed)
	{
		RVec2 CursorPoint = GetMousePositionInViewport();
		SelectSceneObjectAtCursor(CursorPoint);
		bUpdateTransformInWidgets = true;
	}

	if (RInput.GetBufferedKeyState(VK_DELETE) == EBufferedKeyState::Pressed)
	{
		DeleteSelectedObject();
	}

	if (EditorAxis->GetMouseControlMode() != EMouseControlMode::None)
	{
		bUpdateTransformInWidgets = true;
	}

	if (bUpdateTransformInWidgets)
	{
		UpdateWidgetValuesFromObjectTransform();
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
	RScene* DefaultScene = GSceneManager.DefaultScene();

	// Add default camera to scene
	RCamera* Camera = DefaultScene->CreateSceneObjectOfType<RCamera>("Editor Camera", CF_InternalObject | CF_NoSerialization);
	RFreeFlyCameraControl* CameraControl = Camera->AddNewComponent<RFreeFlyCameraControl>();
	CameraControl->SetEnabled(true);

	// Create default lighting
	RSceneObject* GlobalLightInfo = DefaultScene->CreateSceneObjectOfType<RSceneObject>("DirectionalLight", CF_InternalObject | CF_NoSerialization);
	RDirectionalLightComponent* DirLightComponent = GlobalLightInfo->AddNewComponent<RDirectionalLightComponent>();
	DirLightComponent->SetParameters({ RVec3(sinf(1.0f) * 0.5f, 0.25f, cosf(1.0) * 0.5f), RColor(1.0f, 1.0f, 0.8f, 1.0f) });

	EditorAxis = DefaultScene->CreateSceneObjectOfType<REditorAxis>("EditorAxis", CF_InternalObject | CF_NoSerialization);
}

void WorkshopApp::UpdateWidgetValuesFromObjectTransform()
{
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

	EditorAxis->SetSelectedObject(SelectedObject);
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

void WorkshopApp::SelectSceneObjectAtCursor(const RVec2& Point)
{
	auto Cameras = GSceneManager.DefaultScene()->FindAllObjectsOfType<RCamera>();
	if (Cameras.size() == 0)
	{
		return;
	}

	RRay CameraRay = MakeRayFromViewportPoint(Cameras[0], Point);
	EMouseControlMode MouseControlMode = EMouseControlMode::None;

	if (SelectedObject)
	{
		assert(EditorAxis);
		MouseControlMode = EditorAxis->ProcessMouseActions(CameraRay, SelectedObject);
	}

	if (MouseControlMode == EMouseControlMode::None && !RInput.IsKeyDown(VK_LMENU))
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

		// A set of objects hit by the picking ray
		std::vector<RayPickingResult> rayPickingList;
		float tmin = FLT_MAX;

		for (auto SceneObject : GetAllSceneObjects())
		{
			float t;
			if (CameraRay.TestIntersectionWithAabb(SceneObject->GetAabb(), &t))
			{
				// Don't allow picking up a camera or an axis object by click in the scene
				if (SceneObject->CanCastTo<RCamera>() || SceneObject->CanCastTo<REditorAxis>())
				{
					continue;
				}

				if (SceneObject->CanCastTo<RSMeshObject>())
				{
					RSMeshObject* MeshObj = static_cast<RSMeshObject*>(SceneObject);
					for (int i = 0; i < MeshObj->GetMeshElementCount(); i++)
					{
						if (CameraRay.TestIntersectionWithAabb(MeshObj->GetMeshElementAabb(i).GetTransformedAabb(MeshObj->GetTransformMatrix()), &t))
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
			// Unselect any objects
			SetSelectedObject(nullptr);
		}
		else
		{
			// When clicking on a same object more than once, loop through objects in the picking list so objects can be selected even if obstructed
			sort(rayPickingList.begin(), rayPickingList.end(), [](RayPickingResult a, RayPickingResult b) { return a.t < b.t; });
			auto Iter = find(rayPickingList.begin(), rayPickingList.end(), SelectedObject);
			if (Iter != rayPickingList.end())
			{
				Iter++;
				if (Iter == rayPickingList.end())
				{
					Iter = rayPickingList.begin();
				}

				SetSelectedObject(Iter->obj);
			}
			else
			{
				SetSelectedObject(rayPickingList[0].obj);
			}
		}
	}
}

void WorkshopApp::DeleteSelectedObject()
{
	if (SelectedObject)
	{
		RScene* Scene = SelectedObject->GetScene();
		Scene->DestroyObject(SelectedObject);
		SetSelectedObject(nullptr);
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
				// Do not show internal object (editor camera, axis etc)
				if (SceneObjects[i]->HasFlags(CF_InternalObject))
				{
					continue;
				}

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
