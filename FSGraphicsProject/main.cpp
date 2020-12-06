//=============================================================================
// main.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "Rhino.h"

#include "FSGraphicsProjectApp.h"

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	FSGraphicsProjectApp app;
	REngineInitParam InitParam(&app);

	if (GEngine.Initialize(InitParam))
	{
		GEngine.Run();
		GEngine.Shutdown();
	}
	else
	{
		MessageBox(0, L"Failed to initialize REngine.", 0, 0);
	}

	return 0;
}