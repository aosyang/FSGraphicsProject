//=============================================================================
// RAssetEditor_Texture.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetEditor_Texture.h"
#include "RResourcePreviewBuilder.h"

bool RAssetEditor_Texture::IsMatchedAssetType(RResourceBase* Resource) const
{
	return Resource->CastTo<RTexture>() != nullptr;
}

void RAssetEditor_Texture::ShowWindow(RResourceBase* Resource, RResourcePreviewBuilder& PreviewBuilder, RResourceBase* AssetsViewResource)
{
	RTexture* Texture = Resource->CastTo<RTexture>();
	assert(Texture);

	bool bIsSRGB = Texture->GetMetaData()["SRGB"] == "true";
	if (ImGui::Checkbox("sRGB", &bIsSRGB))
	{
		// sRGB check box
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

