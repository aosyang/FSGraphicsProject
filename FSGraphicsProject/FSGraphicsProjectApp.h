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

#include "ConstBufferPS.h"
#include "ConstBufferVS.h"

#define PARTICLE_COUNT 200

enum RenderPass
{
	ShadowPass,
	RefractionScenePass,
	NormalPass,
};

struct PARTICLE_VERTEX
{
	RVec4 pos;
	RVec4 color;
	float	 rot;
};

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
	void CreateSceneRenderTargetView();
	void SetPerObjectConstBuffuer(const RMatrix4& world);
	void RenderSinglePass(RenderPass pass);
	void SetMaterialConstBuffer(SHADER_MATERIAL_BUFFER* buffer);

	bool						m_EnableLights[3];

	RMatrix4					m_CameraMatrix;
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
	ID3D11Buffer*				m_cbScreen;

	ID3D11ShaderResourceView*	m_MeshTextureSRV[3];
	ID3D11SamplerState*			m_SamplerState;
	ID3D11SamplerState*			m_SamplerComparisonState;

	RMesh*						m_SceneMeshCity;
	RSMeshObject				m_FbxMeshObj;

	RMesh*						m_MeshTachikoma;
	RSMeshObject				m_TachikomaObj;
	RShader*					m_RefractionShader;

	RSMeshObject				m_CharacterObj;

	RMesh*						m_AOSceneMesh;
	RSMeshObject				m_AOSceneObj;
	RShader*					m_AOShader;
	ID3D11ShaderResourceView*	m_AOTexture;

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
	PARTICLE_VERTEX				m_ParticleVert[PARTICLE_COUNT];
	ID3D11ShaderResourceView*	m_ParticleDiffuseTexture;
	ID3D11ShaderResourceView*	m_ParticleNormalTexture;
	ID3D11BlendState*			m_BlendState[3];
	ID3D11DepthStencilState*	m_DepthState[2];

	ID3D11Texture2D*			m_RenderTargetBuffer;
	ID3D11RenderTargetView*		m_RenderTargetView;
	ID3D11ShaderResourceView*	m_RenderTargetSRV;
	ID3D11Texture2D*			m_RenderTargetDepthBuffer;
	ID3D11DepthStencilView*		m_RenderTargetDepthView;
};

#endif
