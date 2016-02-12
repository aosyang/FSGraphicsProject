//=============================================================================
// Particle_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "LightShaderCommon.hlsli"

Texture2D DiffuseTexture	: register(t0);
Texture2D NormalTexture		: register(t1);

SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 Color		: COLOR;
	float2 UV			: TEXCOORD0;
	float3 PosW			: TEXCOORD1;
	float4 ShadowPosH	: TEXCOORD2;
	float3 NormalW		: NORMAL;
	float3 TangentW		: TANGENT;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Diffuse = (float4)0;
	float4 Specular = (float4)0;
	float4 Ambient = (float4)0;

	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);

	float3 normal = (NormalTexture.Sample(Sampler, Input.UV) * 2.0f - 1.0f).xyz;
	normal = mul(normal, TBN);

	for (int id = 0; id < DirectionalLightCount; id++)
	{
		float lit = (id == 0) ? SampleShadowMap(ShadowDepthTexture, Sampler, Input.ShadowPosH.xyz) : 1.0f;

		// Diffuse lighting
		Diffuse.rgb += lit * CalculateHalfLambertDiffuseLight(normal, DirectionalLight[id].Direction.xyz, DirectionalLight[id].Color);
	}

	for (int ip = 0; ip < PointLightCount; ip++)
	{
		float3 lightVec = PointLight[ip].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float attenuation = 1.0f - saturate(length(lightVec) / PointLight[ip].PosAndRadius.w);

		// Diffuse lighting
		Diffuse.rgb += attenuation * CalculateHalfLambertDiffuseLight(normal, lightDir, PointLight[ip].Color);
	}

	for (int is = 0; is < SpotlightCount; is++)
	{
		float3 lightVec = Spotlight[is].PosAndInnerConeRatio.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[is].ConeDirAndOuterConeRatio.xyz));
		float attenuation = 1.0f - saturate((Spotlight[is].PosAndInnerConeRatio.w - surfaceRatio) / (Spotlight[is].PosAndInnerConeRatio.w - Spotlight[is].ConeDirAndOuterConeRatio.w));

		// Diffuse lighting
		Diffuse.rgb += attenuation * CalculateHalfLambertDiffuseLight(normal, lightDir, Spotlight[is].Color);
	}

	Diffuse.a = 1.0f;

	Ambient.rgb = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);

	return Input.Color * (Ambient + Diffuse) * DiffuseTexture.Sample(Sampler, Input.UV);
	//return float4(Ambient.xyz, 1.0f);
}
