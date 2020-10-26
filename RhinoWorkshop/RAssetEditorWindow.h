//=============================================================================
// RAssetEditorWindow.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RAssetEditor.h"
#include "EditorCommon.h"

class RResourceBase;
class RResourcePreviewBuilder;

class RAssetEditorWindow : public RGuiWindow
{
public:
	RAssetEditorWindow();

	void SetEditingResource(RResourceBase* NewEditingResource);

protected:
	virtual void OnDrawWindow(REditorContext& EditorContext) override;

private:
	// Current resource being edited
	RResourceBase* EditingResource;

	std::vector<std::unique_ptr<RAssetEditor>> AssetTypeEditors;
};
