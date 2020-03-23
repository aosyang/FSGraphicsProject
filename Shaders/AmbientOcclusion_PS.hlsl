//=============================================================================
// AmbientOcclusion_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightingModel_Phong.hlsli"

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

OUTPUT_PIXEL main(OUTPUT_VERTEX Input)
{
	OUTPUT_PIXEL Out = (OUTPUT_PIXEL)0;
	Out.Albedo = DiffuseTexture.Sample(Sampler, Input.UV0);
	Out.Albedo.a = AmbientOcclusionTexture.Sample(Sampler, Input.UV1).r;
	Out.WorldPos = float4(Input.PosW, Input.PosH.z);
	Out.NormalW = float4(normalize(Input.NormalW), 1);
	Out.NormalV = float4(normalize(Input.NormalV), 1);

	return Out;
}

#else

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float3 normal = normalize(Input.NormalW);
	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

	Input.ShadowPosH[0].xyz /= Input.ShadowPosH[0].w;
	Input.ShadowPosH[1].xyz /= Input.ShadowPosH[1].w;
	Input.ShadowPosH[2].xyz /= Input.ShadowPosH[2].w;

	PhongLightingData LightColor = CalculatePhongLighting(Input.PosW, normal, Input.ShadowPosH, Input.PosH.z, viewDir);

	// Hemisphere ambient color
	float3 Ambient = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor) *
					 AmbientOcclusionTexture.Sample(Sampler, Input.UV1).rgb;

	// Color from diffuse texture
	float4 DiffuseSample = DiffuseTexture.Sample(Sampler, Input.UV0);

	// Compose the final color
	float4 Final;
	Final.rgb = (Ambient + LightColor.Diffuse) * DiffuseSample.rgb + LightColor.Specular;
	Final.a = DiffuseSample.a * GlobalOpacity;

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
		else
		{
			Final *= float4(1, 1, 0, 1);
		}
	}
#endif	// #if DEBUG_CASCADED_LEVELS == 1

	return Final;
}
#endif	// #if USE_DEFERRED_SHADING == 1
