//=============================================================================
// RAssetEditor_Mesh.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RAssetEditor.h"

class RAssetEditor_Mesh : public RAssetEditor
{
public:
	virtual bool IsMatchedAssetType(RResourceBase* Resource) const override;

protected:
	virtual void OnDrawWindow(REditorContext& EditorContext) override;
};
