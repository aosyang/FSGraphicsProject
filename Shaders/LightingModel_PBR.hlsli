//=============================================================================
// LightingModel_PBR.hlsli by Shiyang Ao, 2020 All Rights Reserved.
//
// Physically based lighting model
//=============================================================================

#ifndef _LIGHTING_PBR_HLSLI
#define	_LIGHTING_PBR_HLSLI

#include "LightShaderCommon.hlsli"
#include "BRDF.hlsli"

TextureCube RadianceEnv			: register(t3);
TextureCube IrradianceEnv		: register(t4);

float3 SurfaceColorBRDF(float3 lightDir, float3 lightColor, float3 viewDir, float3 normal, float NdotV, float roughness, float alpha, float3 c_diff, float3 c_spec)
{
	// Half vector of light direction and view direction
	float3 H = normalize(lightDir + viewDir);

	float NdotL = saturate(dot(normal, lightDir));
	float LdotH = saturate(dot(lightDir, H));
	float NdotH = saturate(dot(normal, H));

	float diffuse_factor = Diffuse_Burley(NdotL, NdotV, LdotH, roughness);
	float3 specular = Specular_BRDF(alpha, c_spec, NdotV, NdotL, LdotH, NdotH);

	return NdotL * lightColor * ((c_diff * diffuse_factor) + specular);
}

//////////////////////////////////////////////////////////////////////////
// Calculates light color with PBR
//
// PositionW:				Position in world space
// NormalW:					Normal vector in world space
// ViewDirW:				View vector in world space
// Albedo:					Color of unlit surface
// Roughness:				Roughness of surface material
// Metallic:				Metalness of surface material
// ShadowPosH:				Shadow map sampling position of each cascaded level:
//							xy for texture coordinates from shadow map, z for depth, w is unused
// DepthH:					Position z in homogeneous coordinates
//////////////////////////////////////////////////////////////////////////
float3 CalculatePBRLighting(float3 PositionW, float3 NormalW, float3 ViewDirW, float3 Albedo, float Roughness, float Metallic, float AmbientOcclusion, float4 ShadowPosH[3], float DepthH)
{
	static const float kSpecularCoefficient = 0.04;

	float3 Out = (float3)0;
	float alpha = Roughness * Roughness;
	float NdotV = saturate(dot(NormalW, ViewDirW));

	float3 c_diff = lerp(Albedo, float3(0, 0, 0), Metallic) * AmbientOcclusion;
	float3 c_spec = lerp(kSpecularCoefficient, Albedo, Metallic) * AmbientOcclusion;

	for (int DirLightIdx = 0; DirLightIdx < DirectionalLightCount; DirLightIdx++)
	{
		float lit = 1.0f;
		float3 lightDir = normalize(DirectionalLight[DirLightIdx].Direction.xyz);

		if (DirLightIdx == 0)
		{
			lit = SampleCascadedShadowMap(ShadowPosH, dot(DirectionalLight[DirLightIdx].Direction.xyz, NormalW), DepthH);
		}

		float3 lightColor = DirectionalLight[DirLightIdx].Color.rgb * DirectionalLight[DirLightIdx].Color.w;

		Out += lit * SurfaceColorBRDF(lightDir, lightColor, ViewDirW, NormalW, NdotV, Roughness, alpha, c_diff, c_spec);
	}

	for (int PointLightIdx = 0; PointLightIdx < PointLightCount; PointLightIdx++)
	{
		float3 lightVec = PointLight[PointLightIdx].PosAndRadius.xyz - PositionW;
		float3 lightDir = normalize(lightVec);

		float attenuation = 1.0f - saturate(length(lightVec) / PointLight[PointLightIdx].PosAndRadius.w);

		float3 lightColor = PointLight[PointLightIdx].Color.rgb * PointLight[PointLightIdx].Color.w;

		Out += attenuation * SurfaceColorBRDF(lightDir, lightColor, ViewDirW, NormalW, NdotV, Roughness, alpha, c_diff, c_spec);
	}

	for (int SpotLightIdx = 0; SpotLightIdx < SpotlightCount; SpotLightIdx++)
	{
		float3 lightVec = Spotlight[SpotLightIdx].PosAndRadius.xyz - PositionW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[SpotLightIdx].Direction.xyz));
		float radiusAtt = 1.0f - saturate(length(lightVec) / Spotlight[SpotLightIdx].PosAndRadius.w);
		float coneAtt = 1.0f - saturate((Spotlight[SpotLightIdx].ConeRatio.x - surfaceRatio) / (Spotlight[SpotLightIdx].ConeRatio.x - Spotlight[SpotLightIdx].ConeRatio.y));
		float attenuation = radiusAtt * coneAtt;

		float3 lightColor = Spotlight[SpotLightIdx].Color.rgb * Spotlight[SpotLightIdx].Color.w;

		Out += attenuation * SurfaceColorBRDF(lightDir, lightColor, ViewDirW, NormalW, NdotV, Roughness, alpha, c_diff, c_spec);
	}

	float3 diffuse_env = Diffuse_IBL(IrradianceEnv, NormalW);
	Out += c_diff * diffuse_env;

	float3 specular_env = Specular_IBL(RadianceEnv, NormalW, ViewDirW, Roughness);
	Out += c_spec * specular_env;

	return Out;
}


#endif	// _LIGHTING_PBR_HLSLI
