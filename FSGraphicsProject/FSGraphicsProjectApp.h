//=============================================================================
// FSGraphicsProjectApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _FSGRAPHICSPROJECTAPP_H
#define _FSGRAPHICSPROJECTAPP_H

#include "Rhino.h"
#include "RPostProcessor.h"

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

	bool Initialize();
	void UpdateScene(const RTimer& timer);
	void RenderScene();

	void OnResize(int width, int height);
	TCHAR* WindowTitle() { return L"Graphics Application"; }

private:
	void CreateSceneRenderTargetView();
	void SetPerObjectConstBuffer(const RMatrix4& world);
	void RenderSinglePass(RenderPass pass);
	void SetMaterialConstBuffer(SHADER_MATERIAL_BUFFER* buffer);
	RSphere CalculateFrustumBoundingSphere(const RFrustum& frustum, float start, float end);

	bool						m_EnableLights[3];

	RCamera						m_Camera;
	float						m_CamPitch, m_CamYaw;

	RSkybox						m_Skybox;

	ID3D11InputLayout*			m_ColorPrimitiveIL;
	RMeshRenderBuffer				m_StarMesh;
	RShader*					m_ColorShader;

	RMeshRenderBuffer				m_BumpCubeMesh;
	ID3D11InputLayout*			m_BumpLightingIL;
	RShader*					m_BumpLightingShader;
	RTexture*					m_BumpBaseTexture;
	RTexture*					m_BumpNormalTexture;

	RShaderConstantBuffer<SHADER_OBJECT_BUFFER,		CBST_VS, 0>				m_cbPerObject;
	RShaderConstantBuffer<SHADER_SCENE_BUFFER,		CBST_VS|CBST_GS, 1>		m_cbScene;
	RShaderConstantBuffer<SHADER_INSTANCE_BUFFER,	CBST_VS, 2>				m_cbInstance[3];
	RShaderConstantBuffer<SHADER_SKINNED_BUFFER,	CBST_VS, 3>				m_cbBoneMatrices;
	RShaderConstantBuffer<SHADER_LIGHT_BUFFER,		CBST_PS, 0>				m_cbLight;
	RShaderConstantBuffer<SHADER_MATERIAL_BUFFER,	CBST_PS, 1>				m_cbMaterial;
	RShaderConstantBuffer<SHADER_GLOBAL_BUFFER,		CBST_VS|CBST_PS, 4>		m_cbScreen;

	RMesh*						m_SceneMeshCity;
	RSMeshObject				m_FbxMeshObj;

	RMesh*						m_MeshTachikoma;
	RSMeshObject				m_TachikomaObj;
	RShader*					m_RefractionShader;

	RSMeshObject				m_CharacterObj;
	RMesh*						m_CharacterAnimation;
	vector<RMesh*>				m_CharacterAnimations;
	int							m_CurrentAnim;

	RMesh*						m_AOSceneMesh;
	RSMeshObject				m_AOSceneObj;

	RMesh*						m_SceneMeshIsland;
	RTexture*					m_IslandTexture;
	RSMeshObject				m_IslandMeshObj;

	RShadowMap					m_ShadowMap[3];
	RShader*					m_DepthShader;

	RSMeshObject				m_TransparentMesh;

	RMeshRenderBuffer			m_ParticleBuffer;
	RShader*					m_ParticleShader;
	ID3D11InputLayout*			m_ParticleIL;
	RVertex::PARTICLE_VERTEX	m_ParticleVert[PARTICLE_COUNT];
	RTexture*					m_ParticleDiffuseTexture;
	RTexture*					m_ParticleNormalTexture;
	ID3D11DepthStencilState*	m_DepthState[2];

	ID3D11Texture2D*			m_RenderTargetBuffer;
	ID3D11RenderTargetView*		m_RenderTargetView;
	ID3D11ShaderResourceView*	m_RenderTargetSRV;
	ID3D11Texture2D*			m_RenderTargetDepthBuffer;
	ID3D11DepthStencilView*		m_RenderTargetDepthView;

	RPostProcessor				m_PostProcessor;
	int							m_EnabledPostProcessor;

	RVec3						m_SunVec;
	SHADER_SCENE_BUFFER			cbScene;
	SHADER_LIGHT_BUFFER			cbLight;
	SHADER_INSTANCE_BUFFER		cbInstance[2];

	RDebugRenderer				m_DebugRenderer;
	float						m_CharacterRot;
	float						m_CharacterYVel;
	bool						m_RenderCollisionWireframe;
	RVec4						m_MaterialSpecular;
	int							m_MeshInstanceCount;
};

#endif
