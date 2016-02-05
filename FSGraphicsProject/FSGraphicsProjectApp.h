//=============================================================================
// FSGraphicsProjectApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _FSGRAPHICSPROJECTAPP_H
#define _FSGRAPHICSPROJECTAPP_H

#include "Rhino.h"
#include "RSkybox.h"

#include <vector>
using namespace std;

struct RShader;

class FSGraphicsProjectApp : public IApp
{
public:
	FSGraphicsProjectApp();
	~FSGraphicsProjectApp();

	bool Initialize();
	void UpdateScene(const RTimer& timer);
	void RenderScene();

	//void OnResize(int width, int height) {}
	TCHAR* WindowTitle() { return L"Graphics Application"; }

private:
	void LoadFbxMesh(char* filename);

	XMFLOAT4X4					m_CameraMatrix;
	float						m_CamPitch, m_CamYaw;

	RSkybox						m_Skybox;

	ID3D11InputLayout*			m_ColorPrimitiveIL;
	RMeshElement				m_StarMesh;
	RShader*					m_ColorShader;

	ID3D11InputLayout*			m_LightingMeshIL;
	RShader*					m_LightingShader;

	struct SHADER_SCENE_BUFFER
	{
		XMFLOAT4X4	viewMatrix;
		XMFLOAT4X4	projMatrix;
		XMFLOAT4X4	viewProjMatrix;
		XMFLOAT4	cameraPos;
	};

	ID3D11Buffer*				m_cbPerObject;
	ID3D11Buffer*				m_cbScene;

	vector<RMeshElement>		m_FbxMeshes;
	ID3D11ShaderResourceView*	m_MeshTextureSRV[3];
	ID3D11SamplerState*			m_SamplerState;
};

#endif
