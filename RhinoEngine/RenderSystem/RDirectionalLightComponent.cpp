//=============================================================================
// RDirectionalLightComponent.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RDirectionalLightComponent.h"

const RMatrix4 RDirectionalLightComponent::ShadowBiasTransform =
RMatrix4(
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f);

namespace
{
	float shadowSplitPoints[4] = { 0.0f, 0.05f, 0.4f, 1.0f };

	bool DrawDebugCascadedFrustum = false;
	bool DebugFreezeFrustum = false;
}

RDirectionalLightComponent::RDirectionalLightComponent(RSceneObject* InOwner)
	: Base(InOwner),
	  m_LightDirection(0.0f, 1.0f, 0.0f),
	  m_LightColor(1.0f, 1.0f, 1.0f)
{
	GRenderer.RegisterLight(this);
	GRenderer.RegisterShadowCaster(this);

	for (int i = 0; i < GetDepthPassesNum(); i++)
	{
		m_ShadowMap[i].Initialize(1024, 1024);
	}
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

void RDirectionalLightComponent::PrepareDepthPass(int PassIndex, const RenderViewInfo& View)
{
	assert(PassIndex >= 0 && PassIndex < GetDepthPassesNum());
	assert(View.Frustum);

	auto& cbScene = RConstantBuffers::cbScene.Data;
	RConstantBuffers::cbScene.ClearData();

	// Distance between shadow caster and center of frustum
	float LightDistPerLevel[3] = { 1000.0f, 2000.0f, 2000.0f };

	RFrustum* ViewFrustum = View.Frustum;

	// Freeze view frustum for debugging
	{
		static bool LastDebugState = false;
		if (DebugFreezeFrustum)
		{
			static RFrustum DebugFrustum;

			// Update debug frustum every time when debug freeze frustum is enabled
			if (DebugFreezeFrustum != LastDebugState)
			{
				DebugFrustum = *View.Frustum;
			}

			ViewFrustum = &DebugFrustum;
		}

		LastDebugState = DebugFreezeFrustum;
	}

	// A minimal bounding sphere that covers one slice of the view frustum
	RSphere BoundSphere = CalculateBoundingSphereFromFrustum(*ViewFrustum, shadowSplitPoints[PassIndex], shadowSplitPoints[PassIndex + 1]);

	// Color for debug drawing each level of view frustum
	static RColor FrustumDebugColor[3] = { RColor(1, 0, 0), RColor(0, 1, 0), RColor(0, 0, 1) };

	if (DrawDebugCascadedFrustum)
	{
		GDebugRenderer.DrawSphere(BoundSphere.center, BoundSphere.radius, FrustumDebugColor[PassIndex]);
	}

	// Extend the distance to the light by the bounding sphere's size
	//LightDistPerLevel[PassIndex] = max(LightDistPerLevel[PassIndex], BoundSphere.radius);

	RVec3 shadowTarget = BoundSphere.center;
	RVec3 shadowEyePos = shadowTarget + m_LightDirection * LightDistPerLevel[PassIndex];

	RVec3 viewForward = (shadowTarget - shadowEyePos).GetNormalized();
	RVec3 viewRight = RVec3::Cross(RVec3(0, 1, 0), viewForward).GetNormalized();
	RVec3 viewUp = RVec3::Cross(viewForward, viewRight).GetNormalized();

	// Calculate texel offset in world space
	float texel_unit = BoundSphere.radius * 2.0f / 1024.0f;
	float texel_depth_unit = (BoundSphere.radius + LightDistPerLevel[PassIndex]) / 1024.0f;

	float dx = RVec3::Dot(shadowEyePos, viewRight);
	float dy = RVec3::Dot(shadowEyePos, viewUp);
	float dz = RVec3::Dot(shadowEyePos, viewForward);
	float offset_x = dx - floorf(dx / texel_unit) * texel_unit;
	float offset_y = dy - floorf(dy / texel_unit) * texel_unit;
	float offset_z = dz - floorf(dz / texel_depth_unit) * texel_depth_unit;

	shadowTarget -= viewRight * offset_x + viewUp * offset_y/* + viewForward * offset_z*/;
	shadowEyePos -= viewRight * offset_x + viewUp * offset_y/* + viewForward * offset_z*/;

	RMatrix4 shadowViewMatrix = RMatrix4::CreateLookAtViewLH(shadowEyePos, shadowTarget, RVec3(0.0f, 1.0f, 0.0f));

	RFrustum shadowVolume = m_ShadowMap[PassIndex].GetFrustum();

	if (DrawDebugCascadedFrustum)
	{
		GDebugRenderer.DrawFrustum(shadowVolume, FrustumDebugColor[PassIndex]);
	}

	m_ShadowMap[PassIndex].SetViewMatrix(shadowViewMatrix);
	//m_ShadowMap.SetOrthogonalProjection(500.0f, 500.0f, 0.1f, 5000.0f);
	m_ShadowMap[PassIndex].SetOrthogonalProjection(BoundSphere.radius * 2.0f, BoundSphere.radius * 2.0f, 0.1f, BoundSphere.radius + LightDistPerLevel[PassIndex]);

	RMatrix4 shadowViewProjMatrix = m_ShadowMap[PassIndex].GetViewMatrix() * m_ShadowMap[PassIndex].GetProjectionMatrix();
	cbScene.shadowViewProjMatrix[PassIndex] = shadowViewProjMatrix;
	shadowViewProjMatrix *= ShadowBiasTransform;
	cbScene.shadowViewProjBiasedMatrix[PassIndex] = shadowViewProjMatrix;
	cbScene.cascadedShadowIndex = PassIndex;

	// ShadowViewProjMatrix is required to render depth pass properly
	RConstantBuffers::cbScene.UpdateBufferData();
	RConstantBuffers::cbScene.BindBuffer();

	ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	GRenderer.D3DImmediateContext()->PSSetShaderResources(RShadowMap::ShaderResourceSlot(), 1, nullSRV);

	m_ShadowMap[PassIndex].SetupRenderTarget();
}

void RDirectionalLightComponent::PrepareRenderPass()
{
	auto& cbScene = RConstantBuffers::cbScene.Data;

	for (int i = 0; i < GetDepthPassesNum(); i++)
	{
		RMatrix4 shadowViewProjMatrix = m_ShadowMap[i].GetViewMatrix() * m_ShadowMap[i].GetProjectionMatrix();
		cbScene.shadowViewProjMatrix[i] = shadowViewProjMatrix;
		shadowViewProjMatrix *= ShadowBiasTransform;
		cbScene.shadowViewProjBiasedMatrix[i] = shadowViewProjMatrix;
	}
}

int RDirectionalLightComponent::GetDepthPassesNum() const
{
	// Three-pass cascaded shadow map
	return MAX_CASCADED_SHADOW_SPLITS_NUM;
}

RVec4 RDirectionalLightComponent::GetShadowDepth(RCamera* ViewCamera)
{
	float camNear = ViewCamera->GetNearPlane(),
		  camFar = ViewCamera->GetFarPlane();

	RVec4 sPoints[4] =
	{
		RVec4(0, 0, RMath::Lerp(camNear, camFar, shadowSplitPoints[0]), 1),
		RVec4(0, 0, RMath::Lerp(camNear, camFar, shadowSplitPoints[1]), 1),
		RVec4(0, 0, RMath::Lerp(camNear, camFar, shadowSplitPoints[2]), 1),
		RVec4(0, 0, RMath::Lerp(camNear, camFar, shadowSplitPoints[3]), 1),
	};

	float DepthValues[4] = { 0.0f };

	for (int i = 1; i < 4; i++)
	{
		sPoints[i] = sPoints[i] * ViewCamera->GetProjectionMatrix();
		sPoints[i] /= sPoints[i].w;
		DepthValues[i - 1] = sPoints[i].z;
	}

	return RVec4(DepthValues);
}

RFrustum RDirectionalLightComponent::GetFrustum(int PassIndex)
{
	assert(PassIndex >= 0 && PassIndex < GetDepthPassesNum());
	return m_ShadowMap[PassIndex].GetFrustum();
}

ID3D11ShaderResourceView* RDirectionalLightComponent::GetRTDepthSRV(int PassIndex)
{
	assert(PassIndex >= 0 && PassIndex < GetDepthPassesNum());
	return m_ShadowMap[PassIndex].GetRenderTargetDepthSRV();
}

void RDirectionalLightComponent::SetParameters(const DirectionalLightParam& Parameters)
{
	m_LightDirection = Parameters.Direction.GetNormalized();
	m_LightColor = Parameters.Color;
}

RSphere RDirectionalLightComponent::CalculateBoundingSphereFromFrustum(const RFrustum& frustum, float start, float end)
{
	RVec3 cornerPoints[8] = {
		RVec3::Lerp(frustum.corners[4], frustum.corners[0], start),
		RVec3::Lerp(frustum.corners[5], frustum.corners[1], start),
		RVec3::Lerp(frustum.corners[6], frustum.corners[2], start),
		RVec3::Lerp(frustum.corners[7], frustum.corners[3], start),
		RVec3::Lerp(frustum.corners[4], frustum.corners[0], end),
		RVec3::Lerp(frustum.corners[5], frustum.corners[1], end),
		RVec3::Lerp(frustum.corners[6], frustum.corners[2], end),
		RVec3::Lerp(frustum.corners[7], frustum.corners[3], end),
	};
	RVec3 nearMidPoint = (cornerPoints[0] + cornerPoints[1] + cornerPoints[2] + cornerPoints[3]) / 4.0f;
	RVec3 farMidPoint = (cornerPoints[4] + cornerPoints[5] + cornerPoints[6] + cornerPoints[7]) / 4.0f;
	RVec3 center = (farMidPoint + nearMidPoint) * 0.5f;
	RSphere s = { center, (cornerPoints[4] - center).Magnitude() };

	//if (DrawDebugCascadedFrustum)
	//{
	//	// Center point of the frustum
	//	GDebugRenderer.DrawSphere(center, 50.0f, RColor(1, 1, 0), 8);

	//	// Corner points of the frustum
	//	for (int i = 4; i < 8; i++)
	//	{
	//		GDebugRenderer.DrawSphere(cornerPoints[i], 50.0f, RColor(1, 0, 0), 8);
	//	}
	//}

	return s;
}
