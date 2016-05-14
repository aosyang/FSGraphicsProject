//=============================================================================
// DeferredPointLightPass.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "../Shaders/ConstBufferPS.h"

Texture2D AlbedoTexture		: register(t0);
Texture2D PosDepthTexture	: register(t1);
Texture2D NormalTexture		: register(t2);

SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float2 UV		: TEXCOORD0;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Albedo = AlbedoTexture.Sample(Sampler, Input.UV);
	Albedo.a = 1.0f;

	float3 Normal = NormalTexture.Sample(Sampler, Input.UV).rgb;
	float3 PosW = PosDepthTexture.Sample(Sampler, Input.UV).rgb;

	float3 lightVec = DeferredPointLight.PosAndRadius.xyz - PosW;
	float3 lightDir = normalize(lightVec);

	float attenuation = 1.0f - saturate(length(lightVec) / DeferredPointLight.PosAndRadius.w);
	attenuation *= attenuation;

	// Diffuse lighting
	float DiffuseIntensity = saturate(dot(Normal, lightDir));
	float4 Diffuse = float4((attenuation * DiffuseIntensity * DeferredPointLight.Color.rgb * DeferredPointLight.Color.a), 1.0f);

	// Specular color
	float3 viewDir = normalize(CameraPos.xyz - PosW);
	float3 lightReflect = normalize(reflect(-lightDir, Normal));
	float SpecularIntensity = max(pow(saturate(dot(viewDir, lightReflect)), SpecularColorAndPower.w), 0.0f);
	float4 Specular = (float4)0;
	Specular.rgb = attenuation * DeferredPointLight.Color.rgb * DeferredPointLight.Color.a * SpecularColorAndPower.rgb * SpecularIntensity;

	return Albedo * Diffuse + Specular;
}