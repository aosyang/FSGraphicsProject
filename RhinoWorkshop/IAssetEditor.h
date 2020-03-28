//=============================================================================
// IAssetEditor.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RResourceBase;
class RResourcePreviewBuilder;

struct REditorContext;

class IAssetEditor
{
public:
	virtual bool IsMatchedAssetType(RResourceBase* Resource) const = 0;
	virtual void ShowWindow(REditorContext& EditorContext) = 0;
};
