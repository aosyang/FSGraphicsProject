//=============================================================================
// AssetsViewWindow.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RGuiWindow.h"
#include "Resource/RResourceBase.h"

enum RAssetType
{
	AssetType_Texture	= 1 << 0,
	AssetType_Mesh		= 1 << 1,
	AssetType_Material	= 1 << 2,

	AssetType_All = AssetType_Texture | AssetType_Mesh | AssetType_Material,
};

class RResourcePreviewBuilder;
struct REditorContext;

class RAssetBrowserWindow : public RGuiWindow
{
public:
	RAssetBrowserWindow();

	/// Get the selected resource in assets view
	RResourceBase* GetSelectedResource() const;

	RResourceBase* GetEditingResource() const;

	void SetSelectedResource(RResourceBase* Selection);

	// Set filters for the browser. Will replace existing filter with the new one
	void SetFilter(int FilterBitFlags);

protected:
	/// Draw the window
	virtual void OnDrawWindow(REditorContext& EditorContext) override;

private:
	void DisplayAssetTypeFilter(const char* Label, int FilterType);

	// Bit flags of asset types being displayed in window currently
	int AssetViewFilter;

	// Size of asset preview icons
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

FORCEINLINE void RAssetBrowserWindow::SetFilter(int FilterBitFlags)
{
	AssetViewFilter = FilterBitFlags;
}
