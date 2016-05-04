//=============================================================================
// DeferredShadingApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _DEFERREDSHADINGAPP_H
#define _DEFERREDSHADINGAPP_H

#include "Rhino.h"

class DeferredShadingApp : public IApp
{
public:
	DeferredShadingApp();
	~DeferredShadingApp();

	bool Initialize();
	void UpdateScene(const RTimer& timer);
	void RenderScene();

	void OnResize(int width, int height);
	TCHAR* WindowTitle() { return L"Deferred Shading Demo"; }

private:
	RCamera						m_Camera;
	float						m_CamPitch, m_CamYaw;

	RScene						m_Scene;
	RSMeshObject				m_MeshObj;
};

#endif
