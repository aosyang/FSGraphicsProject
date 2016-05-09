//=============================================================================
// main.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "DeferredShadingApp.h"

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	REngine Engine;
	DeferredShadingApp app;

	Engine.BindApp(&app);

	if (Engine.Initialize())
	{
		Engine.Run();
		Engine.Shutdown();
	}
	else
	{
		MessageBox(0, L"Failed to initialize REngine.", 0, 0);
	}

	return 0;
}