//=============================================================================
// RAssetEditor_Texture.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RAssetEditor_Texture.h"
#include "RResourcePreviewBuilder.h"
#include "RAssetBrowserWindow.h"
#include "EditorCommon.h"

bool RAssetEditor_Texture::CanHandleAssetType(size_t AssetTypeId) const
{
	return AssetTypeId == RTexture::_StaticGetRuntimeTypeId();
}

void RAssetEditor_Texture::OnDrawWindow(REditorContext& EditorContext)
{
	RTexture* Texture = EditorContext.AssetBrowserWindow.GetEditingResource()->CastTo<RTexture>();
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

