//=============================================================================
// RAssetEditor_Material.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetEditor_Material.h"
#include "RResourcePreviewBuilder.h"

bool RAssetEditor_Material::IsMatchedAssetType(RResourceBase* Resource) const
{
	return Resource->CastTo<RMaterial>() != nullptr;
}

void RAssetEditor_Material::ShowWindow(RResourceBase* Resource, RResourcePreviewBuilder& PreviewBuilder, RResourceBase* AssetsViewResource)
{
	RMaterial* Material = Resource->CastTo<RMaterial>();
	assert(Material);

	bool bUpdatePreview = false;

	// Shader selection
	{
		const std::string MaterialShaderName = Material->GetShader()->GetName();
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
	}

	// Material blend mode
	{
		int CurrentBlendMode = (int)Material->GetBlendMode();
		if (ImGui::Combo("Blend Mode", &CurrentBlendMode, BlendStateNames, ARRAYSIZE(BlendStateNames)))
		{
			Material->SetBlendMode((BlendState)CurrentBlendMode);
			bUpdatePreview = true;
		}
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

	{
		auto& Slots = Material->GetTextureSlots();

		ImGui::BeginGroup();
		for (int i = 0; i < (int)Slots.size(); i++)
		{
			const std::string AssignButtonText("->##" + std::to_string(i));
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
		}
		ImGui::EndGroup();

		ImGui::SameLine();
		ImGui::BeginGroup();
		for (int i = 0; i < (int)Slots.size(); i++)
		{
			std::string TextureAssetLabel;
			{
				const RShader* const Shader = Material->GetShader();
				std::string ShaderTextureObjectName;
				if (Shader->QueryTexutreSlotName(i, ShaderTextureObjectName))
				{
					TextureAssetLabel = ShaderTextureObjectName.c_str();
				}
				else
				{
					TextureAssetLabel = "(Unused)";
				}
			}
			TextureAssetLabel = TextureAssetLabel + std::string("##") + std::to_string(i);
			const std::string TextureAssetPath = Slots[i].GetTextureAssetPath();
			char TextureAssetName[256];
			strcpy_s(TextureAssetName, TextureAssetPath.c_str());

			ImGui::SetNextItemWidth(-300);
			if (ImGui::InputText(TextureAssetLabel.c_str(), TextureAssetName, sizeof(TextureAssetName)))
			{
				// TODO: Type texture path and assign it to the material
			}
		}
		ImGui::EndGroup();

		ImGui::SameLine();
		ImGui::BeginGroup();
		for (int i = 0; i < (int)Slots.size(); i++)
		{
			std::string SlotIdLabel = std::string("Slot##") + std::to_string(i);
			std::string SlotIdString = std::to_string(Slots[i].SlotId);
			char TextureSlotName[256];
			strcpy_s(TextureSlotName, SlotIdString.c_str());
			
			ImGui::SetNextItemWidth(-100);
			if (ImGui::InputText(SlotIdLabel.c_str(), TextureSlotName, sizeof(TextureSlotName)))
			{

			}
		}
		ImGui::EndGroup();

		ImGui::SameLine();
		ImGui::BeginGroup();
		for (int i = 0; i < (int)Slots.size(); i++)
		{
			std::string RemoveButtonText("x##" + std::to_string(i));
			if (ImGui::Button(RemoveButtonText.c_str()))
			{
				Material->RemoveTextureSlot(Slots[i].SlotId);
			}
		}
		ImGui::EndGroup();
	}

	// Button for adding a new texture slot
	ImGui::SetNextItemWidth(-1);
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
