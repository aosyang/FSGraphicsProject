//=============================================================================
// FSGraphicsProjectApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _FSGRAPHICSPROJECTAPP_H
#define _FSGRAPHICSPROJECTAPP_H

#include "Rhino.h"

#include <vector>
using namespace std;

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

	ID3D11InputLayout*			m_ColorPrimitiveIL;
	RMeshElement				m_StarMesh;
	ID3D11PixelShader*			m_ColorPixelShader;
	ID3D11VertexShader*			m_ColorVertexShader;

	ID3D11InputLayout*			m_LightingMeshIL;
	ID3D11PixelShader*			m_LightingPixelShader;
	ID3D11VertexShader*			m_LightingVertexShader;

	ID3D11Buffer*				m_cbPerObject;
	ID3D11Buffer*				m_cbScene;

	vector<RMeshElement>		m_FbxMeshes;
	ID3D11ShaderResourceView*	m_MeshTextureSRV;
	ID3D11SamplerState*			m_SamplerState;
};

#endif
