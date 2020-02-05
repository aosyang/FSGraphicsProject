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

	if (ImGui::Begin("Rhino Workshop", nullptr, Flags))
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
