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
struct REditorContext;

class RAssetBrowserWindow
{
public:
	RAssetBrowserWindow();

	/// Draw the window
	void ShowWindow(REditorContext& EditorContext);

	/// Get the selected resource in assets view
	RResourceBase* GetSelectedResource() const;

	RResourceBase* GetEditingResource() const;

	void SetSelectedResource(RResourceBase* Selection);

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

	bool bScrollToSelection;
};

FORCEINLINE RResourceBase* RAssetBrowserWindow::GetSelectedResource() const
{
	return SelectedResource;
}

FORCEINLINE RResourceBase* RAssetBrowserWindow::GetEditingResource() const
{
	return EditingResource;
}