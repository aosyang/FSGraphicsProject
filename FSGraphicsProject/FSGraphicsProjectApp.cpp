//=============================================================================
// FSGraphicsProjectApp.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "FSGraphicsProjectApp.h"


FSGraphicsProjectApp::FSGraphicsProjectApp()
{
}


FSGraphicsProjectApp::~FSGraphicsProjectApp()
{
}

bool FSGraphicsProjectApp::Initialize()
{
	return true;
}

void FSGraphicsProjectApp::UpdateScene(const RTimer& timer)
{

}

void FSGraphicsProjectApp::RenderScene()
{
	RRenderer.Clear();

	RRenderer.Present();
}
