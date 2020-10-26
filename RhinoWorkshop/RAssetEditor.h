//=============================================================================
// IAssetEditor.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RGuiWindow.h"

class RResourceBase;
class RResourcePreviewBuilder;

struct REditorContext;

// Base class for asset editor control. May be used as child elements of other GUI controls.
class RAssetEditor : public RGuiWindow
{
public:
	virtual bool IsMatchedAssetType(RResourceBase* Resource) const { return false; }

protected:
	// Asset editor should always be visible when drawn
	virtual bool IsVisible() override { return true; }
};
