//=============================================================================
// RAssetEditor_Material.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RAssetEditor.h"

class RAssetEditor_Material : public RAssetEditor
{
protected:
	// Begin RAssetEditor method overrides
	virtual bool CanHandleAssetType(size_t AssetTypeId) const override;
	virtual void OnDrawWindow(REditorContext& EditorContext) override;
	// End RAssetEditor method overrides
};
