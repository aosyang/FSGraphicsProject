//=============================================================================
// Lighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

Texture2D DiffuseTexture	: register(t0);
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
	float4 Diffuse = (float4)0;
	float4 Specular = (float4)0;
	float3 normal = normalize(Input.NormalW);
	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

	for (int id = 0; id < DirectionalLightCount; id++)
	{
		// Diffuse lighting
		float DiffuseIntensity = dot(normal, DirectionalLight[id].Direction.xyz);
		Diffuse.rgb += saturate(DiffuseIntensity) * DirectionalLight[id].Color.rgb * DirectionalLight[id].Color.w;

		// Specular lighting
		float3 halfVec = normalize(DirectionalLight[id].Direction.xyz + viewDir);
		float SpecularIntensity = max(pow(saturate(dot(normal, normalize(halfVec))), SpecularColorAndPower.w), 0.0f);
		Specular.rgb += DirectionalLight[id].Color.rgb * SpecularColorAndPower.rgb * SpecularIntensity;
	}

	for (int ip = 0; ip < PointLightCount; ip++)
	{
		// Diffuse lighting
		float3 lightVec = PointLight[ip].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float attenuation = 1.0f - saturate(length(lightVec) / PointLight[ip].PosAndRadius.w);
		float DiffuseIntensity = dot(normal, lightDir);

		Diffuse.rgb += saturate(DiffuseIntensity * attenuation) * PointLight[ip].Color.rgb * PointLight[ip].Color.w;

		// Specular lighting
		float3 halfVec = normalize(lightDir + viewDir);
		float SpecularIntensity = max(pow(saturate(dot(normal, normalize(halfVec))), SpecularColorAndPower.w), 0.0f);
		Specular.rgb += attenuation * PointLight[ip].Color.rgb * SpecularColorAndPower.rgb * SpecularIntensity;
	}

	for (int is = 0; is < SpotlightCount; is++)
	{
		// Diffuse lighting
		float3 lightVec = Spotlight[is].PosAndInnerConeRatio.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[is].ConeDirAndOuterConeRatio.xyz));
		// spotFactor = (surfaceRatio > Spotlight[is].ConeDirAndRatio.w) ? 1.0f : 0.0f;

		float attenuation = 1.0f - saturate((Spotlight[is].PosAndInnerConeRatio.w - surfaceRatio) / (Spotlight[is].PosAndInnerConeRatio.w - Spotlight[is].ConeDirAndOuterConeRatio.w));

		float DiffuseIntensity = dot(normal, lightDir);
		Diffuse.rgb += attenuation * saturate(DiffuseIntensity) * Spotlight[is].Color.rgb * Spotlight[is].Color.w;

		// Specular lighting
		float3 halfVec = normalize(lightDir + viewDir);
		float SpecularIntensity = max(pow(saturate(dot(normal, normalize(halfVec))), SpecularColorAndPower.w), 0.0f);
		Specular.rgb += attenuation * Spotlight[is].Color.rgb * SpecularColorAndPower.rgb * SpecularIntensity;
	}

	//Diffuse.rgb = saturate(Diffuse.rgb);
	Diffuse.a = 1.0f;

	return Diffuse * DiffuseTexture.Sample(Sampler, Input.UV) + Specular;
}
