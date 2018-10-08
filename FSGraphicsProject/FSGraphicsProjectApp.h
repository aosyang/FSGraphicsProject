//=============================================================================
// FSGraphicsProjectApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"

enum EPostProcessingEffect
{
	PPE_GammaCorrection,
	PPE_ColorEdgeDetection,

	PPE_COUNT,
};

#define PARTICLE_COUNT 200

enum RenderPass
{
	ShadowPass,
	RefractionScenePass,
	NormalPass,
};

class FSGraphicsProjectApp : public IApp
{
public:
	FSGraphicsProjectApp();
	~FSGraphicsProjectApp();

	virtual bool Initialize() override;
	virtual void UpdateScene(const RTimer& timer) override;
	virtual void RenderScene() override;

	virtual void OnResize(int width, int height) override;
	virtual TCHAR* WindowTitle() override { return L"Graphics Application"; }

private:
	void CreateSceneRenderTargetView();
	void SetPerObjectConstBuffer(const RMatrix4& world);
	void RenderSinglePass(RenderPass pass);
	void UpdateAndBindMaterialConstBuffer();
	RSphere CalculateFrustumBoundingSphere(const RFrustum& frustum, float start, float end);

	bool						m_EnableLights[3];

	RCamera*					m_Camera;
	float						m_CamPitch, m_CamYaw;

	RScene						m_Scene;
	RSkybox						m_Skybox;

	ID3D11InputLayout*			m_ColorPrimitiveIL;
	RMeshRenderBuffer			m_StarMesh;
	RShader*					m_ColorShader;

	RMeshRenderBuffer			m_BumpCubeMesh;
	ID3D11InputLayout*			m_BumpLightingIL;
	RShader*					m_BumpLightingShader;
	RTexture*					m_BumpBaseTexture;
	RTexture*					m_BumpNormalTexture;

	RShaderConstantBuffer<SHADER_INSTANCE_BUFFER,	CBST_VS, 3>						m_cbInstance[3];

	RMesh*						m_SceneMeshCity;
	RSMeshObject*				m_FbxMeshObj;

	RMesh*						m_MeshTachikoma;
	RSMeshObject*				m_TachikomaObj;
	RShader*					m_RefractionShader;

	RSMeshObject*				m_CharacterObj;
	RMesh*						m_CharacterAnimation;
	vector<RMesh*>				m_CharacterAnimations;
	int							m_CurrentAnim;

	RMesh*						m_AOSceneMesh;
	RSMeshObject*				m_AOSceneObj;

	RMesh*						m_SceneMeshIsland;
	RTexture*					m_IslandTexture;
	RSMeshObject*				m_IslandMeshObj;

	RShadowMap					m_ShadowMap[3];
	RShader*					m_DepthShader;

	RSMeshObject*				m_TransparentMesh;

	RMeshRenderBuffer			m_ParticleBuffer;
	RShader*					m_ParticleShader;
	ID3D11InputLayout*			m_ParticleIL;
	RVertexType::Particle	m_ParticleVert[PARTICLE_COUNT];
	RTexture*					m_ParticleDiffuseTexture;
	RTexture*					m_ParticleNormalTexture;
	ID3D11DepthStencilState*	m_DepthState[2];
	RAabb						m_ParticleAabb;

	ID3D11Texture2D*			m_RenderTargetBuffer;
	ID3D11RenderTargetView*		m_RenderTargetView;
	ID3D11ShaderResourceView*	m_RenderTargetSRV;
	ID3D11Texture2D*			m_RenderTargetDepthBuffer;
	ID3D11DepthStencilView*		m_RenderTargetDepthView;

	RPostProcessingEffect*		m_PostProcessingEffects[PPE_COUNT];
	int							m_EnabledPostProcessor;

	RVec3						m_SunVec;
	SHADER_INSTANCE_BUFFER		cbInstance[2];
	RMatrix4					m_InstanceMatrices[MAX_INSTANCE_COUNT];

	float						m_CharacterRot;
	float						m_CharacterYVel;
	bool						m_RenderCollisionWireframe;
	RVec4						m_MaterialSpecular;
	int							m_MeshInstanceCount;
};

