//=============================================================================
// FSGraphicsProjectApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _FSGRAPHICSPROJECTAPP_H
#define _FSGRAPHICSPROJECTAPP_H

#include "Rhino.h"
#include "RSkybox.h"
#include "RShadowMap.h"

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
	void SetPerObjectConstBuffuer(const XMMATRIX& world);

	bool						m_EnableLights[3];

	XMFLOAT4X4					m_CameraMatrix;
	float						m_CamPitch, m_CamYaw;

	RSkybox						m_Skybox;

	ID3D11InputLayout*			m_ColorPrimitiveIL;
	RMeshElement				m_StarMesh;
	RShader*					m_ColorShader;

	RMeshElement				m_BumpCubeMesh;
	ID3D11InputLayout*			m_BumpLightingIL;
	RShader*					m_BumpLightingShader;
	ID3D11ShaderResourceView*	m_BumpBaseTextureSRV;
	ID3D11ShaderResourceView*	m_BumpNormalTextureSRV;

	ID3D11InputLayout*			m_LightingMeshIL;
	RShader*					m_LightingShader;

	ID3D11Buffer*				m_cbPerObject;
	ID3D11Buffer*				m_cbScene;
	ID3D11Buffer*				m_cbLight;
	ID3D11Buffer*				m_cbMaterial;
	ID3D11Buffer*				m_cbInstance;

	ID3D11ShaderResourceView*	m_MeshTextureSRV[3];
	ID3D11SamplerState*			m_SamplerState;
	ID3D11SamplerState*			m_SamplerComparisonState;

	RMesh*						m_SceneMeshCity;
	RSMeshObject				m_FbxMeshObj;

	RMesh*						m_SceneMeshIsland;
	ID3D11ShaderResourceView*	m_IslandTextureSRV;
	RSMeshObject				m_IslandMeshObj;
	RShader*					m_InstancedLightingShader;

	RShadowMap					m_ShadowMap;
	RShader*					m_DepthShader;
	RShader*					m_InstancedDepthShader;

	RMeshElement				m_ParticleBuffer;
	RShader*					m_ParticleShader;
	ID3D11InputLayout*			m_ParticleIL;
	XMFLOAT4					m_ParticlePos[1000];
	XMFLOAT4					m_ParticleColor[1000];
};

#endif
