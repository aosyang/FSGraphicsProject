//=============================================================================
// RAssetEditorWindow.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetEditorWindow.h"

#include "RResourcePreviewBuilder.h"

RAssetEditorWindow::RAssetEditorWindow()
	: bShowWindow(false)
	, EditingResource(nullptr)
{

}

void RAssetEditorWindow::SetEditingResource(RResourceBase* NewEditingResource)
{
	EditingResource = NewEditingResource;
}

void RAssetEditorWindow::ShowWindow(RResourcePreviewBuilder& PreviewBuilder, RResourceBase* AssetsViewResource)
{
	if (bShowWindow)
	{
		if (ImGui::Begin("Asset Editor", nullptr, ImGuiWindowFlags_MenuBar))
		{
			if (EditingResource)
			{
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::MenuItem("Save Asset"))
						{
						}

						if (RMesh* Mesh = EditingResource->CastTo<RMesh>())
						{
							if (ImGui::MenuItem("Save Default Materials"))
							{
								Mesh->SaveMaterialsToDiskAsDefaults();
							}
						}

						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}

				std::string AssetNameText("Asset: " + EditingResource->GetAssetPath());
				ImGui::Text(AssetNameText.c_str());

				if (RMesh* Mesh = EditingResource->CastTo<RMesh>())
				{
					const auto& Materials = Mesh->GetMaterials();

					for (int i = 0; i < Mesh->GetMeshElementCount(); i++)
					{
						std::string AssignButtonText("->##" + std::to_string(i));
						if (ImGui::Button(AssignButtonText.c_str()))
						{
							if (auto Resource = AssetsViewResource)
							{
								if (RMaterial* MaterialAsset = Resource->CastTo<RMaterial>())
								{
									Mesh->SetMaterialSlot(i, MaterialAsset);
									PreviewBuilder.BuildPreviewForResource(Mesh);
								}
							}
						}

						ImGui::SameLine();

						std::string MaterialAssetPath;
						std::string Label = "Slot " + std::to_string(i) + ": " + Mesh->GetMeshElements()[i].GetName();

						if (i < (int)Materials.size())
						{
							MaterialAssetPath = Materials[i]->GetAssetPath();
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
}
