//=============================================================================
// RAssetEditor_Material.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "IAssetEditor.h"

class RAssetEditor_Material : public IAssetEditor
{
public:
	virtual bool IsMatchedAssetType(RResourceBase* Resource) const override;
	virtual void ShowWindow(RResourceBase* Resource, RResourcePreviewBuilder& PreviewBuilder, RResourceBase* AssetsViewResource) override;
};
