//=============================================================================
// RAssetEditorWindow.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetEditorWindow.h"

#include "RAssetBrowserWindow.h"
#include "RResourcePreviewBuilder.h"
#include "RAssetEditor_Mesh.h"
#include "RAssetEditor_Texture.h"
#include "RAssetEditor_Material.h"

RAssetEditorWindow::RAssetEditorWindow()
	: EditingResource(nullptr)
{
	AssetTypeEditors.push_back(std::make_unique<RAssetEditor_Mesh>());
	AssetTypeEditors.push_back(std::make_unique<RAssetEditor_Texture>());
	AssetTypeEditors.push_back(std::make_unique<RAssetEditor_Material>());
}

void RAssetEditorWindow::SetEditingResource(RResourceBase* NewEditingResource)
{
	EditingResource = NewEditingResource;
}

void RAssetEditorWindow::OnDrawWindow(REditorContext& EditorContext)
{
	if (ImGui::Begin("Asset Editor", &bShowWindow, ImGuiWindowFlags_MenuBar))
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

			// Find editor for asset type and draw the interface
			for (auto& Editor : AssetTypeEditors)
			{
				if (Editor->CanHandleAssetType(EditingResource->GetRuntimeTypeId()))
				{
					Editor->DrawWindow(EditorContext);
					break;
				}
			}

			// Draw preview images

			// Textures can be drawn directly as previews
			RTexture* PreviewTexture = EditingResource->CastTo<RTexture>();

			// Find preview textures for non-texture type resources
			if (PreviewTexture == nullptr)
			{
				PreviewTexture = EditorContext.PreviewBuilder.FindPreviewTexture(EditingResource);
			}

			if (PreviewTexture)
			{
				ImGui::Text("Preview");
				ImGui::Image(PreviewTexture->GetSRV(), ImVec2(256, 256));
			}
		}
	}

	ImGui::End();
}
