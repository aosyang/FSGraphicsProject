//=============================================================================
// RDirectionalLightComponent.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RDirectionalLightComponent.h"

RDirectionalLightComponent::RDirectionalLightComponent(RSceneObject* InOwner)
	: Base(InOwner),
	  m_Direction(0.0f, 1.0f, 0.0f),
	  m_Color(1.0f, 1.0f, 1.0f)
{
	GRenderer.RegisterLight(this);
	GRenderer.RegisterShadowCaster(this);

	m_ShadowMap.Initialize(1024, 1024);
}

RDirectionalLightComponent::~RDirectionalLightComponent()
{
	GRenderer.UnregisterShadowCaster(this);
	GRenderer.UnregisterLight(this);
}

RDirectionalLightComponent* RDirectionalLightComponent::Create(RSceneObject* InOwner)
{
	RDirectionalLightComponent* DirectionalLightComponent = new RDirectionalLightComponent(InOwner);
	return DirectionalLightComponent;
}

ELightType RDirectionalLightComponent::GetLightType()
{
	return ELightType::DirectionalLight;
}

bool RDirectionalLightComponent::CanCastShadow()
{
	return true;
}

void RDirectionalLightComponent::PrepareShadowPass()
{
	RVec3 LightPos = GetDirection() * 300.0f;
	RMatrix4 shadowViewMatrix = RMatrix4::CreateLookAtViewLH(LightPos, RVec3(0.0f, 0.0f, 0.0f), RVec3(0.0f, 1.0f, 0.0f));

	m_ShadowMap.SetViewMatrix(shadowViewMatrix);
	m_ShadowMap.SetOrthogonalProjection(500.0f, 500.0f, 0.1f, 800.0f);

	SHADER_SCENE_BUFFER cbScene;
	ZeroMemory(&cbScene, sizeof(cbScene));

	RMatrix4 shadowTransform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	RMatrix4 shadowViewProjMatrix = m_ShadowMap.GetViewMatrix() * m_ShadowMap.GetProjectionMatrix();
	cbScene.shadowViewProjMatrix[0] = shadowViewProjMatrix;
	shadowViewProjMatrix *= shadowTransform;
	cbScene.shadowViewProjBiasedMatrix[0] = shadowViewProjMatrix;

	// ShadowViewProjMatrix is required to render depth pass properly
	RConstantBuffers::cbScene.UpdateBufferData(&cbScene);
	RConstantBuffers::cbScene.BindBuffer();

	ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	GRenderer.D3DImmediateContext()->PSSetShaderResources(RShadowMap::ShaderResourceSlot(), 1, nullSRV);

	m_ShadowMap.SetupRenderTarget();
}

void RDirectionalLightComponent::PrepareRenderPass(SHADER_SCENE_BUFFER* SceneConstBuffer)
{
	RMatrix4 shadowTransform(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	RMatrix4 shadowViewProjMatrix = m_ShadowMap.GetViewMatrix() * m_ShadowMap.GetProjectionMatrix();
	SceneConstBuffer->shadowViewProjMatrix[0] = shadowViewProjMatrix;
	shadowViewProjMatrix *= shadowTransform;
	SceneConstBuffer->shadowViewProjBiasedMatrix[0] = shadowViewProjMatrix;
}

RFrustum RDirectionalLightComponent::GetFrustum()
{
	return m_ShadowMap.GetFrustum();
}

ID3D11ShaderResourceView* RDirectionalLightComponent::GetRTDepthSRV()
{
	return m_ShadowMap.GetRenderTargetDepthSRV();
}

void RDirectionalLightComponent::SetParameters(const DirectionalLightParam& Parameters)
{
	m_Direction = Parameters.Direction;
	m_Color = Parameters.Color;
}

