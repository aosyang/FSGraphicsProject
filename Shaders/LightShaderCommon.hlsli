//=============================================================================
// LightShaderCommon.hlsli by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _LIGHTSHADERCOMMON_HLSLI
#define _LIGHTSHADERCOMMON_HLSLI

Texture2D ShadowDepthTexture : register(t2);
SamplerComparisonState ShadowMapComparisonState : register(s2);

float3x3 CalculateTBNSpace(float3 NormalW, float3 TangentW)
{
	float3 N = normalize(NormalW);
	float3 T = normalize(TangentW);
	float3 B = cross(N, T);
	T = cross(B, N);

	return float3x3(T, B, N);
}

float3 CalculateDiffuseLight(float3 normal,
							 float3 lightDir,
							 float4 lightColor)
{
	float DiffuseIntensity = dot(normal, lightDir);
	return saturate(DiffuseIntensity) * lightColor.rgb * lightColor.a;
}

float3 CalculateHalfLambertDiffuseLight(float3 normal,
										float3 lightDir,
										float4 lightColor)
{
	float DiffuseIntensity = dot(normal, lightDir) / 2.0f + 0.5f;
	return saturate(DiffuseIntensity) * lightColor.rgb * lightColor.a;
}

float3 CalculateAmbientLight(float3 normal, float4 highHemisphereColor, float4 lowHemisphereColor)
{
	return saturate(dot(normal, float3(0.0f, 1.0f, 0.0f)) / 2.0f + 0.5f) * highHemisphereColor.rgb * highHemisphereColor.w +
		   saturate(dot(normal, float3(0.0f, -1.0f, 0.0f)) / 2.0f + 0.5f) * lowHemisphereColor.rgb * lowHemisphereColor.w;
}

float3 CalculateSpecularLight(float3 normal,
							  float3 lightDir,
							  float3 viewDir,
							  float4 lightColor,
							  float4 specularColor)
{
	//float3 halfVec = normalize(lightDir + viewDir);
	//float SpecularIntensity = max(pow(saturate(dot(normal, normalize(halfVec))), specularColor.w), 0.0f);
	float3 lightReflect = normalize(reflect(-lightDir, normal));
	float SpecularIntensity = max(pow(saturate(dot(viewDir, lightReflect)), specularColor.w), 0.0f);
	return lightColor.rgb * specularColor.rgb * SpecularIntensity;
}

float SampleShadowMap(Texture2D shadowMap, SamplerState shadowMapSampler, float3 shadowPosH)
{
	// Nearest shadow map sampling
	//return (shadowPosH.z < ShadowDepthTexture.Sample(shadowMapSampler, shadowPosH.xy).r + 0.0001f) ? 1.0f : 0.0f;

	// Bilinear filtered shadow map comparison
	//return shadowMap.SampleCmpLevelZero(ShadowMapComparisonState, shadowPosH.xy, shadowPosH.z - 0.01f);

	// 4x4 PCF 
	float final = 0;
	for (float x = -1.5f; x <= 1.5f; x++)
	{
		for (float y = -1.5f; y <= 1.5f; y++)
		{
			float2 offset = float2(x, y) / 1024.0f;
			final += shadowMap.SampleCmpLevelZero(ShadowMapComparisonState, shadowPosH.xy + offset, shadowPosH.z - 0.01f) / 16.0f;
		}
	}

	return final;
}

#endif