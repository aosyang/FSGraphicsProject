//=============================================================================
// RAssetEditor_Mesh.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetEditor_Mesh.h"
#include "RResourcePreviewBuilder.h"

bool RAssetEditor_Mesh::IsMatchedAssetType(RResourceBase* Resource) const
{
	return Resource->CastTo<RMesh>() != nullptr;
}

void RAssetEditor_Mesh::ShowWindow(RResourceBase* Resource, RResourcePreviewBuilder& PreviewBuilder, RResourceBase* AssetsViewResource)
{
	RMesh* Mesh = Resource->CastTo<RMesh>();
	assert(Mesh != nullptr);

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

