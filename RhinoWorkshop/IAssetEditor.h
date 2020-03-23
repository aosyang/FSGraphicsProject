//=============================================================================
// IAssetEditor.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RResourceBase;
class RResourcePreviewBuilder;

class IAssetEditor
{
public:
	virtual bool IsMatchedAssetType(RResourceBase* Resource) const = 0;
	virtual void ShowWindow(RResourceBase* Resource, RResourcePreviewBuilder& PreviewBuilder, RResourceBase* AssetsViewResource) = 0;
};
