//=============================================================================
// WorkshopApp.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "WorkshopApp.h"

#include "Rhino.h"
#include "../RhinoGameUtils/RFreeFlyCameraControl.h"

WorkshopApp::WorkshopApp()
	: bShowOpenDialog(false)
	, bShowAssetsView(false)
	, AssetViewFilter(AssetType_All)
	, SelectedObject(nullptr)
	, SelectedResource(nullptr)
{

}

bool WorkshopApp::Initialize()
{
	RResourceManager::Instance().LoadAllResources(EResourceLoadMode::Immediate);
	MeshPreviewBuilder.BuildPreviewForAllMeshes();

	GSceneManager.DefaultScene()->Initialize();

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

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Show Assets View", nullptr, &bShowAssetsView);
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

		DisplaySceneViewWindow();
		DisplayAssetsViewWindow();

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
						RMesh* MeshAsset = SelectedResource->CastTo<RMesh>();
						if (MeshAsset)
						{
							MeshObject->SetMesh(MeshAsset);
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

void WorkshopApp::DisplayAssetsViewWindow()
{
	if (bShowAssetsView)
	{
		if (ImGui::Begin("Assets View"))
		{
			ImGui::Text("Filter:");
			DisplayAssertFilter("All", AssetType_All); ImGui::SameLine();
			DisplayAssertFilter("Mesh", AssetType_Mesh); ImGui::SameLine();
			DisplayAssertFilter("Texture", AssetType_Texture);

			const char* PreviewSizeOptions[] = { "512", "256", "128", "64" };
			static const char* CurrentPreviewSize = PreviewSizeOptions[1];
			if (ImGui::BeginCombo("View size", CurrentPreviewSize))
			{
				for (int i = 0; i < ARRAYSIZE(PreviewSizeOptions); i++)
				{
					bool bSelected = (CurrentPreviewSize == PreviewSizeOptions[i]);
					if (ImGui::Selectable(PreviewSizeOptions[i], bSelected))
					{
						CurrentPreviewSize = PreviewSizeOptions[i];
					}
					if (bSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			int PreviewSize = atoi(CurrentPreviewSize);

			ImGui::BeginChild("AssetChild");
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			ImGuiStyle& style = ImGui::GetStyle();

			auto& ResourceList = RResourceManager::Instance().EnumerateAllResources();
			for (auto Iter : ResourceList)
			{
				if (Iter->CanCastTo<RMesh>() && !(AssetViewFilter & AssetType_Mesh))
				{
					continue;
				}

				if (Iter->CanCastTo<RTexture>() && !(AssetViewFilter & AssetType_Texture))
				{
					continue;
				}

				ImVec2 ItemSize((float)PreviewSize, (float)PreviewSize);

				// Position of selectable widget
				ImVec2 p = ImGui::GetCursorScreenPos();
				
				// Hide label text from selectable
				std::string HiddenLabel = "##" + Iter->GetAssetPath();
				const char* Label = Iter->GetAssetPath().c_str();
				ImVec2 LabelSize = ImGui::CalcTextSize(Label);

				ImTextureID GuiTexture = nullptr;
				if (RTexture* Texture = Iter->CastTo<RTexture>())
				{
					// ImGui doesn't handle cubemap rendering. Skip them for now
					if (!Texture->IsCubeMap())
					{
						GuiTexture = Texture->GetSRV();
					}
				}
				else if (RMesh* Mesh = Iter->CastTo<RMesh>())
				{
					RTexture* PreviewTexture = MeshPreviewBuilder.FindPreviewTexture(Mesh);
					if (PreviewTexture)
					{
						GuiTexture = PreviewTexture->GetSRV();
					}
				}

				if (ImGui::Selectable(HiddenLabel.c_str(), SelectedResource == Iter, 0, ImVec2(ItemSize.x, ItemSize.y + LabelSize.y + 2)))
				{
					SelectedResource = Iter;
				}
				float last_button_x2 = ImGui::GetItemRectMax().x;

				// Draw custom texture and text for the selectable
				ImGui::GetWindowDrawList()->AddImage(GuiTexture, ImVec2(p.x + 2, p.y + 2), ImVec2(p.x + ItemSize.x + 2, p.y + ItemSize.y + 2));
				ImGui::GetWindowDrawList()->AddText(ImVec2(p.x, p.y + ItemSize.y + 2), ImGui::GetColorU32(ImGuiCol_Text), Label);

				// Position of next widget
				p = ImGui::GetCursorScreenPos();

				// If there is room for another icon then draw it in the same line
				float next_button_x2 = last_button_x2 + style.ItemSpacing.x + ItemSize.x;
				if (next_button_x2 < window_visible_x2)
				{
					ImGui::SameLine();
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
}

void WorkshopApp::DisplayAssertFilter(const char* Label, int FilterType)
{
	if (FilterType == AssetType_All)
	{
		bool bFilterChecked = (AssetViewFilter == FilterType);
		ImGui::Checkbox(Label, &bFilterChecked);
		if (bFilterChecked != (AssetViewFilter == FilterType))
		{
			if (bFilterChecked)
			{
				AssetViewFilter = AssetType_All;
			}
			else
			{
				AssetViewFilter = 0;
			}
		}
	}
	else
	{
		bool bFilterChecked = (AssetViewFilter & FilterType);
		ImGui::Checkbox(Label, &bFilterChecked);
		if (bFilterChecked)
		{
			AssetViewFilter |= FilterType;
		}
		else
		{
			AssetViewFilter &= ~FilterType;
		}
	}
}
