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
	, Selected(nullptr)
{

}

bool WorkshopApp::Initialize()
{
	GSceneManager.DefaultScene()->Initialize();

	return true;
}

void WorkshopApp::UpdateScene(const RTimer& timer)
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2((float)WindowWidth, 200));

	ImGuiWindowFlags Flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBackground |
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
				ImGui::MenuItem("New Map");

				// Open menu
				if (ImGui::MenuItem("Open"))
				{
					// Note: Since the menu code path won't execute when the popup is open, we need to indicate the state by a variable
					bShowOpenDialog = true;
					UpdateEngineMapList();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
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

					Selected = nullptr;
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
	}

	if (Selected)
	{
		GDebugRenderer.DrawAabb(Selected->GetAabb());
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
	RCamera* Camera = DefaultScene->CreateSceneObjectOfType<RCamera>();
	RFreeFlyCameraControl* CameraControl = Camera->AddNewComponent<RFreeFlyCameraControl>();
	CameraControl->SetEnabled(true);
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
				Selected = rayPickingList[0].obj;
			}
		}
	}
}
