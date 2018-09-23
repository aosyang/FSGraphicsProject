//=============================================================================
// IApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Engine application interface
//=============================================================================

#pragma once

class IApp
{
public:
	virtual ~IApp() {}

	virtual bool Initialize() = 0;
	virtual void UpdateScene(const RTimer& timer) = 0;
	virtual void RenderScene() = 0;

	virtual void OnResize(int width, int height) {}
	virtual TCHAR* WindowTitle() { return L"Rhino Engine"; }
};

