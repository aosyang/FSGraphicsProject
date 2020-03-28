//=============================================================================
// EditorCommon.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

RVec2 GetMousePositionInViewport();

RRay MakeRayFromViewportPoint(RCamera* Camera, const RVec2& Point);

// Context object for accessing editor windows from other windows
struct REditorContext
{
	class RAssetEditorWindow& AssetEditorWindow;
	class RAssetBrowserWindow& AssetBrowserWindow;
	class RResourcePreviewBuilder& PreviewBuilder;
};
