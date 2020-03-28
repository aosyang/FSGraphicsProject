//=============================================================================
// RAssetEditor_Mesh.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "IAssetEditor.h"

class RAssetEditor_Mesh : public IAssetEditor
{
public:
	virtual bool IsMatchedAssetType(RResourceBase* Resource) const override;
	virtual void ShowWindow(REditorContext& EditorContext) override;
};
