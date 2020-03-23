//=============================================================================
// LightingModel_Phong.hlsli by Shiyang Ao, 2016 All Rights Reserved.
//
// Phong lighting model
//=============================================================================

#ifndef _LIGHTING_PHONE_HLSLI
#define	_LIGHTING_PHONE_HLSLI

#include "LightShaderCommon.hlsli"

#ifndef PHONG_NO_SPECULAR
#define PHONG_NO_SPECULAR 0
#endif	// ifndef PHONG_NO_SPECULAR

struct PhongLightingData
{
	float3 Diffuse;

#if PHONG_NO_SPECULAR == 0
	float3 Specular;
#endif	// PHONG_NO_SPECULAR == 0
};

//////////////////////////////////////////////////////////////////////////
// Calculates lighting with Phone model
//
// PositionW:				Position in world space
// NormalW:					Normal vector in world space
// ShadowPosH:				Shadow map sampling position of each cascaded level:
//							xy for texture coordinates from shadow map, z for depth, w is unused
// DepthH:					Position z in homogeneous coordinates
// ViewDirW:				View vector in world space (unused if PHONG_NO_SPECULAR is defined)
// InSpecularColorPower:	The Specular color and power. If not specified, the default value from
//							the constant buffer which is set globally in the engine will be used
//							(unused if PHONG_NO_SPECULAR is defined)
//////////////////////////////////////////////////////////////////////////
PhongLightingData CalculatePhongLighting(float3 PositionW, float3 NormalW, float4 ShadowPosH[3], float DepthH, float3 ViewDirW = 0, float4 InSpecularColorPower = float4(-1, -1, -1, -1))
{
	PhongLightingData Out = (PhongLightingData)0;

#if PHONG_NO_SPECULAR == 0
	// Use the default specular color and power value if we didn't specify one 
	InSpecularColorPower = InSpecularColorPower.w < 0 ? SpecularColorAndPower : InSpecularColorPower;
#endif	// PHONG_NO_SPECULAR == 0

	for (int DirLightIdx = 0; DirLightIdx < DirectionalLightCount; DirLightIdx++)
	{
		float lit = 1.0f;

		if (DirLightIdx == 0)
		{
			lit = SampleCascadedShadowMap(ShadowPosH, dot(DirectionalLight[DirLightIdx].Direction.xyz, NormalW), DepthH);
		}

		// Diffuse lighting
		Out.Diffuse += lit * CalculateDiffuseLight(NormalW, DirectionalLight[DirLightIdx].Direction.xyz, DirectionalLight[DirLightIdx].Color);

#if PHONG_NO_SPECULAR == 0
		// Specular lighting
		Out.Specular += lit * CalculateSpecularLight(NormalW, DirectionalLight[DirLightIdx].Direction.xyz, ViewDirW, DirectionalLight[DirLightIdx].Color, InSpecularColorPower);
#endif	// PHONG_NO_SPECULAR == 0
	}

	for (int PointLightIdx = 0; PointLightIdx < PointLightCount; PointLightIdx++)
	{
		float3 lightVec = PointLight[PointLightIdx].PosAndRadius.xyz - PositionW;
		float3 lightDir = normalize(lightVec);

		float attenuation = 1.0f - saturate(length(lightVec) / PointLight[PointLightIdx].PosAndRadius.w);

		// Diffuse lighting
		Out.Diffuse += attenuation * CalculateDiffuseLight(NormalW, lightDir, PointLight[PointLightIdx].Color);

#if PHONG_NO_SPECULAR == 0
		// Specular lighting
		Out.Specular += attenuation * CalculateSpecularLight(NormalW, lightDir, ViewDirW, PointLight[PointLightIdx].Color, InSpecularColorPower);
#endif	// PHONG_NO_SPECULAR == 0
	}

	for (int SpotLightIdx = 0; SpotLightIdx < SpotlightCount; SpotLightIdx++)
	{
		float3 lightVec = Spotlight[SpotLightIdx].PosAndRadius.xyz - PositionW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[SpotLightIdx].Direction.xyz));
		float radiusAtt = 1.0f - saturate(length(lightVec) / Spotlight[SpotLightIdx].PosAndRadius.w);
		float coneAtt = 1.0f - saturate((Spotlight[SpotLightIdx].ConeRatio.x - surfaceRatio) / (Spotlight[SpotLightIdx].ConeRatio.x - Spotlight[SpotLightIdx].ConeRatio.y));
		float attenuation = radiusAtt * coneAtt;

		// Diffuse lighting
		Out.Diffuse += attenuation * CalculateDiffuseLight(NormalW, lightDir, Spotlight[SpotLightIdx].Color);

#if PHONG_NO_SPECULAR == 0
		// Specular lighting
		Out.Specular += attenuation * CalculateSpecularLight(NormalW, lightDir, ViewDirW, Spotlight[SpotLightIdx].Color, InSpecularColorPower);
#endif	// PHONG_NO_SPECULAR == 0
	}

	return Out;
}

#endif	// _LIGHTING_PHONE_HLSLI
