//=============================================================================
// InstancedLighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "LightShaderCommon.hlsli"

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
		Diffuse.rgb += CalculateDiffuseLight(normal, DirectionalLight[id].Direction.xyz, DirectionalLight[id].Color);

		// Specular lighting
		Specular.rgb += CalculateSpecularLight(normal, DirectionalLight[id].Direction.xyz, viewDir, DirectionalLight[id].Color, SpecularColorAndPower);
	}

	for (int ip = 0; ip < PointLightCount; ip++)
	{
		float3 lightVec = PointLight[ip].PosAndRadius.xyz - Input.PosW;
			float3 lightDir = normalize(lightVec);

			float attenuation = 1.0f - saturate(length(lightVec) / PointLight[ip].PosAndRadius.w);

		// Diffuse lighting
		Diffuse.rgb += attenuation * CalculateDiffuseLight(normal, lightDir, PointLight[ip].Color);

		// Specular lighting
		Specular.rgb += attenuation * CalculateSpecularLight(normal, lightDir, viewDir, PointLight[ip].Color, SpecularColorAndPower);
	}

	for (int is = 0; is < SpotlightCount; is++)
	{
		float3 lightVec = Spotlight[is].PosAndInnerConeRatio.xyz - Input.PosW;
			float3 lightDir = normalize(lightVec);

			float surfaceRatio = saturate(dot(-lightDir, Spotlight[is].ConeDirAndOuterConeRatio.xyz));
		float attenuation = 1.0f - saturate((Spotlight[is].PosAndInnerConeRatio.w - surfaceRatio) / (Spotlight[is].PosAndInnerConeRatio.w - Spotlight[is].ConeDirAndOuterConeRatio.w));

		// Diffuse lighting
		Diffuse.rgb += attenuation * CalculateDiffuseLight(normal, lightDir, Spotlight[is].Color);

		// Specular lighting
		Specular.rgb += attenuation * CalculateSpecularLight(normal, lightDir, viewDir, Spotlight[is].Color, SpecularColorAndPower);
	}

	Diffuse.a = 1.0f;

	return Diffuse * DiffuseTexture.Sample(Sampler, Input.UV) + Specular;
}
