//=============================================================================
// RAssetEditorWindow.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "IAssetEditor.h"
#include "EditorCommon.h"

class RResourceBase;
class RResourcePreviewBuilder;

class RAssetEditorWindow
{
public:
	RAssetEditorWindow();

	void SetEditingResource(RResourceBase* NewEditingResource);

	void ShowWindow(REditorContext& EditorContext);

	/// Visibility of asset editor window
	bool bShowWindow;
private:

	RResourceBase* EditingResource;

	std::vector<std::unique_ptr<IAssetEditor>> AssetTypeEditors;
};
