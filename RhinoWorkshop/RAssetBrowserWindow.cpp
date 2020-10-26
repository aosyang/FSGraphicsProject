//=============================================================================
// AssetsViewWindow.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetBrowserWindow.h"

#include "RResourcePreviewBuilder.h"
#include "RAssetEditorWindow.h"
#include "EditorCommon.h"

RAssetBrowserWindow::RAssetBrowserWindow()
	: AssetViewFilter(AssetType_All)
	, PreviewIconSize(256)
	, SelectedResource(nullptr)
	, EditingResource(nullptr)
	, bShowNewMaterialDialog(false)
	, bScrollToSelection(false)
{
	strcpy_s(NewMaterialPath, "/");
}

void RAssetBrowserWindow::OnDrawWindow(REditorContext& EditorContext)
{
	if (ImGui::Begin("Asset Browser", &bShowWindow, ImGuiWindowFlags_MenuBar))
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
				EditorContext.PreviewBuilder.BuildPreviewForResource(NewMaterial);

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

		ImGui::Text("Show Types:");
		DisplayAssetTypeFilter("All", AssetType_All); ImGui::SameLine();
		DisplayAssetTypeFilter("Mesh", AssetType_Mesh); ImGui::SameLine();
		DisplayAssetTypeFilter("Texture", AssetType_Texture); ImGui::SameLine();
		DisplayAssetTypeFilter("Material", AssetType_Material);

		// Radio buttons for sizes of icons
		ImGui::Text("Icon Size:");
		for (int i = 5; i <= 10; i++)
		{
			int Val = (int)powf(2.0f, (float)i);
			ImGui::SameLine();
			ImGui::RadioButton(std::to_string(Val).c_str(), &PreviewIconSize, Val);
		}

		// The search bar
		AssetNameFilter.Draw("Filter");
		ImGui::SameLine();
		if (ImGui::Button("Clear"))
		{
			AssetNameFilter.Clear();
		}

		ImGui::BeginChild("AssetChild");
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
		ImGuiStyle& style = ImGui::GetStyle();

		int NumTotalItemLines = 0;
		int SelectedLineIdx = -1;
		ImVec2 SelectedItemCursorPos;

		auto& ResourceList = RResourceManager::Instance().EnumerateAllResources();
		for (auto Iter : ResourceList)
		{
			// Asset type filtering
			if ((Iter->CanCastTo<RMesh>() && !(AssetViewFilter & AssetType_Mesh)) ||
				(Iter->CanCastTo<RTexture>() && !(AssetViewFilter & AssetType_Texture)) ||
				(Iter->CanCastTo<RMaterial>() && !(AssetViewFilter & AssetType_Material)))
			{
				continue;
			}

			// Search bar filtering
			if (!AssetNameFilter.PassFilter(Iter->GetAssetPath().c_str()))
			{
				continue;
			}

			ImVec2 ItemSize((float)PreviewIconSize, (float)PreviewIconSize);

			// Position of selectable widget
			ImVec2 ItemScreenPos = ImGui::GetCursorScreenPos();
			ImVec2 ItemWindowPos = ImGui::GetCursorPos();

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
				RTexture* PreviewTexture = EditorContext.PreviewBuilder.FindPreviewTexture(Iter);
				if (PreviewTexture)
				{
					GuiTexture = PreviewTexture->GetSRV();
				}
			}

			// Click an asset item to select it
			if (ImGui::Selectable(HiddenLabel.c_str(), SelectedResource == Iter, 0, ImVec2(ItemSize.x, ItemSize.y + LabelSize.y + 2)))
			{
				SelectedResource = Iter;
			}

			// Double-click an asset item to open its editor window
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				// Request open asset editor
				EditorContext.AssetEditorWindow.bShowWindow = true;
				EditingResource = SelectedResource;
			}

			float last_button_x2 = ImGui::GetItemRectMax().x;

			// Draw custom texture and text for the selectable
			ImGui::GetWindowDrawList()->AddImage(GuiTexture, ImVec2(ItemScreenPos.x + 2, ItemScreenPos.y + 2), ImVec2(ItemScreenPos.x + ItemSize.x + 2, ItemScreenPos.y + ItemSize.y + 2));
			ImGui::GetWindowDrawList()->AddText(ImVec2(ItemScreenPos.x, ItemScreenPos.y + ItemSize.y + 2), ImGui::GetColorU32(ImGuiCol_Text), Label);

			// Position of next widget
			ItemScreenPos = ImGui::GetCursorScreenPos();

			if (SelectedResource == Iter)
			{
				SelectedLineIdx = NumTotalItemLines;
				SelectedItemCursorPos = ItemWindowPos;
			}

			// If there is room for another icon then draw it in the same line
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + ItemSize.x;
			if (next_button_x2 < window_visible_x2)
			{
				ImGui::SameLine();
			}
			else
			{
				NumTotalItemLines++;
			}
		}

		// Scroll the browser to show the selected asset
		if (bScrollToSelection)
		{
			if (SelectedLineIdx != -1)
			{
				const float ScrollRatio = (float)SelectedLineIdx / (float)NumTotalItemLines;

				ImVec2 RectSize = ImGui::GetItemRectSize();
				float CursorY = ImGui::GetContentRegionMax().y;

				// Note: SetScrollHereY is not working
				//ImGui::SetScrollHereY(ScrollRatio);
				float ScrollMaxY = ImGui::GetScrollMaxY();
				ImGui::SetScrollY(ScrollMaxY * ScrollRatio);
			}

			bScrollToSelection = false;
		}
		ImGui::EndChild();
	}
	ImGui::End();
}

void RAssetBrowserWindow::SetSelectedResource(RResourceBase* Selection)
{
	SelectedResource = Selection;
	bScrollToSelection = true;
}

void RAssetBrowserWindow::DisplayAssetTypeFilter(const char* Label, int FilterType)
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

