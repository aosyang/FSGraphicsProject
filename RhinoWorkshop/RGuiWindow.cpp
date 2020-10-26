//=============================================================================
// RGuiWindow.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RGuiWindow.h"

RGuiWindow::RGuiWindow()
	: bShowWindow(false)
{

}

RGuiWindow::~RGuiWindow()
{

}

void RGuiWindow::DrawWindow(REditorContext& EditorContext)
{
	if (IsVisible())
	{
		OnDrawWindow(EditorContext);
	}
}

