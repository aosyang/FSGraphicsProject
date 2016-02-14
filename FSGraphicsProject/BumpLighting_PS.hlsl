//=============================================================================
// BumpLighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
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
	float4 PosH			: SV_POSITION;
	float2 UV			: TEXCOORD0;
	float3 PosW			: TEXCOORD1;
	float3 NormalW		: NORMAL;
	float3 TangentW		: TANGENT;
	float4 ShadowPosH	: TEXCOORD3;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Diffuse = (float4)0;
	float4 Specular = (float4)0;
	float4 Ambient = (float4)0;

	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);

	float3 normal = (NormalTexture.Sample(Sampler, Input.UV) * 2.0f - 1.0f).xyz;
	normal = mul(normal, TBN);

	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

	for (int id = 0; id < DirectionalLightCount; id++)
	{
		float lit = (id == 0) ? SampleShadowMap(ShadowDepthTexture, Sampler, Input.ShadowPosH.xyz) : 1.0f;

		// Diffuse lighting
		Diffuse.rgb += lit * CalculateDiffuseLight(normal, DirectionalLight[id].Direction.xyz, DirectionalLight[id].Color);

		// Specular lighting
		Specular.rgb += lit * CalculateSpecularLight(normal, DirectionalLight[id].Direction.xyz, viewDir, DirectionalLight[id].Color, SpecularColorAndPower);
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
		float3 lightVec = Spotlight[is].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[is].Direction.xyz));
		float radiusAtt = 1.0f - saturate(length(lightVec) / Spotlight[is].PosAndRadius.w);
		float coneAtt = 1.0f - saturate((Spotlight[is].ConeRatio.x - surfaceRatio) / (Spotlight[is].ConeRatio.x - Spotlight[is].ConeRatio.y));
		float attenuation = radiusAtt * coneAtt;

		// Diffuse lighting
		Diffuse.rgb += attenuation * CalculateDiffuseLight(normal, lightDir, Spotlight[is].Color);

		// Specular lighting
		Specular.rgb += attenuation * CalculateSpecularLight(normal, lightDir, viewDir, Spotlight[is].Color, SpecularColorAndPower);
	}

	Diffuse.a = 1.0f;

	Ambient.rgb = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);

	return (Ambient + Diffuse) * DiffuseTexture.Sample(Sampler, Input.UV) + Specular;
}
