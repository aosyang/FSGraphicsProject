//=============================================================================
// RGuiWindow.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

struct REditorContext;

// GUI window base class
class RGuiWindow
{
public:
	RGuiWindow();
	virtual ~RGuiWindow();

	// Called when a GUI window is required to be drawn
	void DrawWindow(REditorContext& EditorContext);

	// Visibility of window
	// Made public so imgui have access to it.
	bool bShowWindow;

protected:
	virtual void OnDrawWindow(REditorContext& EditorContext) {}
	virtual bool IsVisible() { return bShowWindow; }
};
