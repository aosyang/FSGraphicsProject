//=============================================================================
// AmbientOcclusion_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightShaderCommon.hlsli"

Texture2D DiffuseTexture			: register(t0);
Texture2D AmbientOcclusionTexture	: register(t1);

struct OUTPUT_VERTEX
{
	float4 PosH				: SV_POSITION;
	float2 UV0				: TEXCOORD0;
	float2 UV1				: TEXCOORD1;
	float3 PosW				: TEXCOORD2;
	float3 NormalV			: TEXCOORD3;
	float4 ShadowPosH[3]	: TEXCOORD4;
	float3 NormalW			: NORMAL;
};

#define DEBUG_CASCADED_LEVELS 0

#if USE_DEFERRED_SHADING == 1

struct OUTPUT_PIXEL
{
	float4 Albedo		: SV_Target0;
	float4 WorldPos		: SV_Target1;
	float4 NormalW		: SV_Target2;
	float4 NormalV		: SV_Target3;
};

OUTPUT_PIXEL main(OUTPUT_VERTEX Input) : SV_TARGET
{
	OUTPUT_PIXEL Out = (OUTPUT_PIXEL)0;
	Out.Albedo = MakeLinearColorFromGammaSpace(DiffuseTexture.Sample(Sampler, Input.UV0));
	Out.WorldPos = float4(Input.PosW, Input.PosH.z);
	Out.NormalW = float4(normalize(Input.NormalW), 1);
	Out.NormalV = float4(normalize(Input.NormalV), 1);

	return Out;
}

#else

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Diffuse = (float4)0;
	float4 Specular = (float4)0;
	float4 Ambient = (float4)0;
	float3 normal = normalize(Input.NormalW);
	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

	Input.ShadowPosH[0].xyz /= Input.ShadowPosH[0].w;
	Input.ShadowPosH[1].xyz /= Input.ShadowPosH[1].w;
	Input.ShadowPosH[2].xyz /= Input.ShadowPosH[2].w;

	for (int id = 0; id < DirectionalLightCount; id++)
	{
		float lit = 1.0f;

		if (id == 0)
		{
			lit = SampleCascadedShadowMap(Input.ShadowPosH, dot(DirectionalLight[id].Direction.xyz, normal), Input.PosH.z);
		}

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

	Ambient.rgb = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor) *
				  AmbientOcclusionTexture.Sample(Sampler, Input.UV1).rgb;

	float4 Final = (Ambient + Diffuse) * MakeLinearColorFromGammaSpace(DiffuseTexture.Sample(Sampler, Input.UV0)) + Specular;
	Final.a *= GlobalOpacity;

#if DEBUG_CASCADED_LEVELS == 1
	if (CascadedShadowCount > 1)
	{
		float depth = Input.PosH.z;

		float4 level0 = float4(1, 0, 0, 1);
		float4 level1 = float4(0, 1, 0, 1);
		float4 level2 = float4(0, 0, 1, 1);

		if (depth < CascadedShadowDepth[0])
		{
			float blend = saturate((depth - CascadedShadowDepth[0] * 0.9998) / 0.0002);
			Final *= lerp(level0, level1, blend);
		}
		else if (depth < CascadedShadowDepth[1])
		{
			float blend = saturate((depth - CascadedShadowDepth[1] * 0.9999) / 0.0001);
			Final *= lerp(level1, level2, blend);
		}
		else if (CascadedShadowCount > 2 && depth < CascadedShadowDepth[2])
		{
			float blend = saturate((depth - CascadedShadowDepth[2] * 0.99995) / 0.00005);
			Final *= lerp(level2, 1.0f, blend);
		}
	}
#endif	// #if DEBUG_CASCADED_LEVELS == 1

	return Final;
}
#endif	// #if USE_DEFERRED_SHADING == 1
