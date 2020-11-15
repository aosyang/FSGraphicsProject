//=============================================================================
// IApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Engine application interface
//=============================================================================

#pragma once

#include "RTimer.h"
#include <tchar.h>

class IApp
{
public:
	virtual ~IApp() {}

	virtual bool Initialize() = 0;
	virtual void UpdateScene(const RTimer& timer) = 0;
	
	// If returns true, instead of using engine rendering fuction IApp::RenderScene will be called.
	// Override this function in a derived app class if you need a customized rendering process.
	virtual bool UsingCustomRenderPipeline() { return false; }

	// User implementation of scene rendering. This function will not be called if IsUsingCustomRenderPipeline is false.
	virtual void RenderScene() {}

	virtual void OnResize(int width, int height) {}
	virtual TCHAR* WindowTitle() { return L"Rhino Engine"; }
};

