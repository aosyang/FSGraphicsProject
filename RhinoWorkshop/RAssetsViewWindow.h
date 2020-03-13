//=============================================================================
// AssetsViewWindow.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Resource/RResourceBase.h"

enum RAssetType
{
	AssetType_Texture = 1 << 0,
	AssetType_Mesh = 1 << 1,
	AssetType_Material = 1 << 2,

	AssetType_All = AssetType_Texture | AssetType_Mesh | AssetType_Material,
};

class RResourcePreviewBuilder;

class RAssetsViewWindow
{
public:
	RAssetsViewWindow();

	/// Draw the window
	void ShowWindow(RResourcePreviewBuilder& PreviewBuilder, bool& ShowAssetEditor);

	/// Get the selected resource in assets view
	RResourceBase* GetSelectedResource() const;

	RResourceBase* GetEditingResource() const;

	/// Visibility of asset view window
	bool bShowWindow;
private:
	void DisplayAssetTypeFilter(const char* Label, int FilterType);

	int AssetViewFilter;
	int PreviewIconSize;

	RResourceBase* SelectedResource;
	RResourceBase* EditingResource;

	ImGuiTextFilter AssetNameFilter;

	bool bShowNewMaterialDialog;
	char NewMaterialName[MAX_PATH];
	char NewMaterialPath[MAX_PATH];
};

FORCEINLINE RResourceBase* RAssetsViewWindow::GetSelectedResource() const
{
	return SelectedResource;
}

FORCEINLINE RResourceBase* RAssetsViewWindow::GetEditingResource() const
{
	return EditingResource;
}
