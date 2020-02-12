//=============================================================================
// AssetsViewWindow.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetsViewWindow.h"

#include "RResourcePreviewBuilder.h"

RAssetsViewWindow::RAssetsViewWindow()
	: bShowWindow(false)
	, AssetViewFilter(AssetType_All)
	, PreviewIconSize(256)
	, SelectedResource(nullptr)
	, EditingResource(nullptr)
	, bShowNewMaterialDialog(false)
{
	strcpy_s(NewMaterialPath, "/");
}

void RAssetsViewWindow::ShowWindow(RResourcePreviewBuilder& PreviewBuilder, bool& ShowAssetEditor)
{
	if (bShowWindow)
	{
		if (ImGui::Begin("Assets View", nullptr, ImGuiWindowFlags_MenuBar))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("New Material"))
					{
						bShowNewMaterialDialog = true;
						ZeroMemory(NewMaterialName, sizeof(NewMaterialName));
					}

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			if (bShowNewMaterialDialog)
			{
				ImGui::OpenPopup("New Material");
			}

			if (ImGui::BeginPopupModal("New Material", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::InputText("Material name", NewMaterialName, sizeof(NewMaterialName));
				ImGui::InputText("Asset path", NewMaterialPath, sizeof(NewMaterialPath));
				if (ImGui::Button("Create"))
				{
					std::string MaterialName(NewMaterialName);
					std::string AssetPath(NewMaterialPath);

					// Add extension to material name
					MaterialName = RFileUtil::StripExtension(MaterialName) + ".material";

					// Replace back slashes with forward slashes
					std::replace(AssetPath.begin(), AssetPath.end(), '\\', '/');
					if (AssetPath[AssetPath.size() - 1] != '/')
					{
						AssetPath += '/';
					}

					RMaterial* NewMaterial = RResourceManager::Instance().CreateNewResource<RMaterial>(AssetPath + MaterialName);
					PreviewBuilder.BuildPreviewForResource(NewMaterial);

					ImGui::CloseCurrentPopup();
					bShowNewMaterialDialog = false;
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					bShowNewMaterialDialog = false;
				}

				ImGui::EndPopup();
			}

			ImGui::Text("Filter:");
			DisplayAssertFilter("All", AssetType_All); ImGui::SameLine();
			DisplayAssertFilter("Mesh", AssetType_Mesh); ImGui::SameLine();
			DisplayAssertFilter("Texture", AssetType_Texture); ImGui::SameLine();
			DisplayAssertFilter("Material", AssetType_Material);

			// The slider for icon size
			ImGui::SliderInt("Icon Size", &PreviewIconSize, 16, 1024);

			ImGui::BeginChild("AssetChild");
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			ImGuiStyle& style = ImGui::GetStyle();

			auto& ResourceList = RResourceManager::Instance().EnumerateAllResources();
			for (auto Iter : ResourceList)
			{
				if ((Iter->CanCastTo<RMesh>() && !(AssetViewFilter & AssetType_Mesh)) ||
					(Iter->CanCastTo<RTexture>() && !(AssetViewFilter & AssetType_Texture)) ||
					(Iter->CanCastTo<RMaterial>() && !(AssetViewFilter & AssetType_Material)))
				{
					continue;
				}

				ImVec2 ItemSize((float)PreviewIconSize, (float)PreviewIconSize);

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
				else
				{
					RTexture* PreviewTexture = PreviewBuilder.FindPreviewTexture(Iter);
					if (PreviewTexture)
					{
						GuiTexture = PreviewTexture->GetSRV();
					}
				}

				if (ImGui::Selectable(HiddenLabel.c_str(), SelectedResource == Iter, 0, ImVec2(ItemSize.x, ItemSize.y + LabelSize.y + 2)))
				{
					SelectedResource = Iter;
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					// Request open asset editor
					ShowAssetEditor = true;
					EditingResource = SelectedResource;
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

void RAssetsViewWindow::DisplayAssertFilter(const char* Label, int FilterType)
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

