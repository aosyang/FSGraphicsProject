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
							EditingResource->SaveToDisk();
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
				else if (RMaterial* Material = EditingResource->CastTo<RMaterial>())
				{
					bool bUpdatePreview = false;

					std::string MaterialShaderName = Material->GetShader()->GetName();
					int CurrentShader = 0;

					auto ShaderNames = RShaderManager::Instance().EnumerateAllShaderNames();
					std::vector<const char*> ShaderNameStrings;
					for (int i = 0; i < (int)ShaderNames.size(); i++)
					{
						ShaderNameStrings.push_back(ShaderNames[i].c_str());
						if (ShaderNames[i] == MaterialShaderName)
						{
							CurrentShader = i;
						}
					}

					if (ImGui::Combo("Shader", &CurrentShader, ShaderNameStrings.data(), (int)ShaderNameStrings.size()))
					{
						Material->SetShader(RShaderManager::Instance().GetShaderResource(ShaderNameStrings[CurrentShader]));
						bUpdatePreview = true;
					}

					int CurrentBlendMode = (int)Material->GetBlendMode();
					if (ImGui::Combo("Blend Mode", &CurrentBlendMode, BlendStateNames, ARRAYSIZE(BlendStateNames)))
					{
						Material->SetBlendMode((BlendState)CurrentBlendMode);
						bUpdatePreview = true;
					}

					bool bDoubleSided = Material->GetDoubleSided();
					if (ImGui::Checkbox("Double Sided", &bDoubleSided))
					{
						Material->SetDoubleSided(bDoubleSided);
						bUpdatePreview = true;
					}

					float UVTiling = Material->GetUVTiling();
					if (ImGui::InputFloat("UV Tiling", &UVTiling))
					{
						Material->SetUVTiling(UVTiling);
						bUpdatePreview = true;
					}

					auto& Slots = Material->GetTextureSlots();
					for (int i = 0; i < (int)Slots.size(); i++)
					{
						std::string AssignButtonText("->##" + std::to_string(i));
						if (ImGui::Button(AssignButtonText.c_str()))
						{
							if (auto Resource = AssetsViewResource)
							{
								if (RTexture* TextureAsset = Resource->CastTo<RTexture>())
								{
									Material->SetTextureSlot(Slots[i].SlotId, TextureAsset);
									bUpdatePreview = true;
								}
							}
						}
						ImGui::SameLine();
				
						std::string TextureAssetLabel = std::string("Texture##") + std::to_string(i);
						std::string TextureAssetPath = Slots[i].GetTextureAssetPath();
						char TextureAssetName[256];
						strcpy_s(TextureAssetName, TextureAssetPath.c_str());
						ImGui::SetNextItemWidth(-200);
						if (ImGui::InputText(TextureAssetLabel.c_str(), TextureAssetName, sizeof(TextureAssetName)))
						{

						}
						ImGui::SameLine();

						std::string SlotIdLabel = std::string("Slot##") + std::to_string(i);
						std::string SlotIdString = std::to_string(Slots[i].SlotId);
						char TextureSlotName[256];
						strcpy_s(TextureSlotName, SlotIdString.c_str());
						ImGui::SetNextItemWidth(-100);
						if (ImGui::InputText(SlotIdLabel.c_str(), TextureSlotName, sizeof(TextureSlotName)))
						{

						}
						ImGui::SameLine();

						std::string RemoveButtonText("x##" + std::to_string(i));
						if (ImGui::Button(RemoveButtonText.c_str()))
						{
							Material->RemoveTextureSlot(Slots[i].SlotId);
						}
					}

					// Button for adding a new texture slot
					if (ImGui::Button("+"))
					{
						int MaxSlot = -1;
						for (auto& Slot : Material->GetTextureSlots())
						{
							if (Slot.SlotId > MaxSlot)
							{
								MaxSlot = Slot.SlotId;
							}
						}

						Material->SetTextureSlot(MaxSlot + 1, nullptr);
					}

					if (bUpdatePreview)
					{
						PreviewBuilder.BuildPreviewForResource(Material);
					}
				}
				else if (RTexture* Texture = EditingResource->CastTo<RTexture>())
				{
					bool bIsSRGB = Texture->GetMetaData()["SRGB"] == "true";
					if (ImGui::Checkbox("sRGB", &bIsSRGB))
					{
						if (bIsSRGB)
						{
							Texture->GetMetaData().AddAttribute("SRGB", "true");
						}
						else
						{
							Texture->GetMetaData().RemoveAttribute("SRGB");
						}

						// Reload the texture to reflect the change
						Texture->SaveToDisk();
						Texture->Reload();
					}
				}

				// Draw the preview
				RTexture* PreviewTexture = EditingResource->CanCastTo<RTexture>() ? EditingResource->CastTo<RTexture>() : PreviewBuilder.FindPreviewTexture(EditingResource);
				if (PreviewTexture)
				{
					ImGui::Text("Preview");
					ImGui::Image(PreviewTexture->GetSRV(), ImVec2(256, 256));
				}
			}
		}

		ImGui::End();
	}
}
