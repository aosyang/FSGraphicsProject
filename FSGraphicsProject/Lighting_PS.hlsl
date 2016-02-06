//=============================================================================
// Lighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

Texture2D DiffuseTexture;
SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 Color	: COLOR;
	float4 PosH		: SV_POSITION;
	float2 UV		: TEXCOORD0;
	float3 NormalW	: TEXCOORD1;
	float3 PosW		: TEXCOORD2;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 lightColor = (float4)0;
	float3 normal = normalize(Input.NormalW);

	for (int id = 0; id < DirectionalLightCount; id++)
	{
		float intensity = dot(normal, DirectionalLight[id].Direction.xyz);
		lightColor.rgb += saturate(intensity) * DirectionalLight[id].Color.rgb * DirectionalLight[id].Color.w;
	}

	for (int ip = 0; ip < PointLightCount; ip++)
	{
		float3 lightVec = PointLight[ip].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float attenuation = 1.0f - saturate(length(lightVec) / PointLight[ip].PosAndRadius.w);
		float intensity = dot(normal, lightDir);

		lightColor.rgb += saturate(intensity * attenuation) * PointLight[ip].Color.rgb * PointLight[ip].Color.w;
	}

	for (int is = 0; is < SpotlightCount; is++)
	{
		float3 lightVec = Spotlight[is].PosAndInnerConeRatio.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[is].ConeDirAndOuterConeRatio.xyz));
		// spotFactor = (surfaceRatio > Spotlight[is].ConeDirAndRatio.w) ? 1.0f : 0.0f;

		float attenuation = 1.0f - saturate((Spotlight[is].PosAndInnerConeRatio.w - surfaceRatio) / (Spotlight[is].PosAndInnerConeRatio.w - Spotlight[is].ConeDirAndOuterConeRatio.w));

		float intensity = dot(normal, lightDir);
		lightColor.rgb += attenuation * saturate(intensity) * Spotlight[is].Color.rgb * Spotlight[is].Color.w;
	}

	//lightColor.rgb = saturate(lightColor.rgb);
	lightColor.a = 1.0f;

	return lightColor * DiffuseTexture.Sample(Sampler, Input.UV);
}
