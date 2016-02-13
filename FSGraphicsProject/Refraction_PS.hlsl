//=============================================================================
// Refraction_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "LightShaderCommon.hlsli"

Texture2D ScreenTexture	: register(t0);
SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH			: SV_POSITION;
	float4 UV			: TEXCOORD0;
	float3 NormalH		: TEXCOORD1;
	float3 PosW			: TEXCOORD2;
	float4 ShadowPosH	: TEXCOORD3;
	float3 NormalW		: NORMAL;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Specular = (float4)0;
	float4 Ambient = (float4)0;
	float3 normal = normalize(Input.NormalW);
	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

	Input.ShadowPosH.xyz /= Input.ShadowPosH.w;

	for (int id = 0; id < DirectionalLightCount; id++)
	{
		float lit = (id == 0) ? SampleShadowMap(ShadowDepthTexture, Sampler, Input.ShadowPosH.xyz) : 1.0f;

		// Specular lighting
		Specular.rgb += lit * CalculateSpecularLight(normal, DirectionalLight[id].Direction.xyz, viewDir, DirectionalLight[id].Color, SpecularColorAndPower);
	}

	for (int ip = 0; ip < PointLightCount; ip++)
	{
		float3 lightVec = PointLight[ip].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float attenuation = 1.0f - saturate(length(lightVec) / PointLight[ip].PosAndRadius.w);

		// Specular lighting
		Specular.rgb += attenuation * CalculateSpecularLight(normal, lightDir, viewDir, PointLight[ip].Color, SpecularColorAndPower);
	}

	for (int is = 0; is < SpotlightCount; is++)
	{
		float3 lightVec = Spotlight[is].PosAndInnerConeRatio.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[is].ConeDirAndOuterConeRatio.xyz));
		float attenuation = 1.0f - saturate((Spotlight[is].PosAndInnerConeRatio.w - surfaceRatio) / (Spotlight[is].PosAndInnerConeRatio.w - Spotlight[is].ConeDirAndOuterConeRatio.w));

		// Specular lighting
		Specular.rgb += attenuation * CalculateSpecularLight(normal, lightDir, viewDir, Spotlight[is].Color, SpecularColorAndPower);
	}

	float2 uvOffset = float2(Input.NormalH.x / ScreenSize.x, Input.NormalH.y / ScreenSize.y) * 64.0f;
	float4 distortion = ScreenTexture.Sample(Sampler, Input.UV.xy / Input.UV.w * 0.5f + 0.5f + uvOffset);
	float4 color = float4(0.75f, 1.0f, 0.8f, 1.0f);
	float ambientRim = 1.0f - saturate(dot(Input.NormalH, float3(0, 0, -1)));
	Ambient.rgb = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);
	Ambient.a = 1.0f;

	return lerp(distortion, Ambient, ambientRim)* color + Specular;
}