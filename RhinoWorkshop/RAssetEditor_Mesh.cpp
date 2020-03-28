//=============================================================================
// RAssetEditor_Mesh.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetEditor_Mesh.h"
#include "RResourcePreviewBuilder.h"
#include "RAssetBrowserWindow.h"
#include "EditorCommon.h"

bool RAssetEditor_Mesh::IsMatchedAssetType(RResourceBase* Resource) const
{
	return Resource->CastTo<RMesh>() != nullptr;
}

void RAssetEditor_Mesh::ShowWindow(REditorContext& EditorContext)
{
	RMesh* Mesh = EditorContext.AssetBrowserWindow.GetEditingResource()->CastTo<RMesh>();
	assert(Mesh != nullptr);

	const auto& Materials = Mesh->GetMaterials();

	for (int i = 0; i < Mesh->GetMeshElementCount(); i++)
	{
		std::string AssignButtonText("->##" + std::to_string(i));
		if (ImGui::Button(AssignButtonText.c_str()))
		{
			if (auto Resource = EditorContext.AssetBrowserWindow.GetSelectedResource())
			{
				if (RMaterial* MaterialAsset = Resource->CastTo<RMaterial>())
				{
					Mesh->SetMaterialSlot(i, MaterialAsset);
					EditorContext.PreviewBuilder.BuildPreviewForResource(Mesh);
				}
			}
		}

		ImGui::SameLine();

		std::string MaterialAssetPath;
		std::string Label = "Slot " + std::to_string(i) + ": " + Mesh->GetMeshElements()[i].GetName();
		
		if (i < (int)Materials.size() && Materials[i])
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

