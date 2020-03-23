//=============================================================================
// BumpLightingPBR_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"

#include "LightingModel_PBR.hlsli"

Texture2D AlbedoTexture			: register(t0);
Texture2D NormalTexture			: register(t1);
Texture2D RoughnessMetallicAO	: register(t2);

struct OUTPUT_VERTEX
{
	float4 Color			: COLOR;
	float4 PosH				: SV_POSITION;
	float2 UV				: TEXCOORD0;
	float3 PosW				: TEXCOORD1;
	float3 NormalW			: NORMAL;
	float3 TangentW			: TANGENT;
	float4 ShadowPosH[3]	: TEXCOORD2;
};

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
	Out.Albedo = AlbedoTexture.Sample(Sampler, GetTiledUV(Input.UV));
	Out.WorldPos = float4(Input.PosW, Input.PosH.z);

	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);
	float3 normal = normalize((NormalTexture.Sample(Sampler, GetTiledUV(Input.UV)) * 2.0f - 1.0f).xyz);
	float3 worldNormal = mul(normal, TBN);

	Out.NormalW = float4(worldNormal, 1);
	Out.NormalV = float4(mul(worldNormal, (float3x3)viewMatrix), 1);

	return Out;
}

#else

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	[unroll]
	for (int i = 0; i < 3; i++)
	{
		Input.ShadowPosH[i] /= Input.ShadowPosH[i].w;
	}

	// Sample roughtness, metallic and AO from rgb channels
	float3 RMA = RoughnessMetallicAO.Sample(Sampler, GetTiledUV(Input.UV)).rgb;
	float roughness = RMA.r;
	float metallic = RMA.g;
	float ambientOcclusion = RMA.b;

	float3 albedo = AlbedoTexture.Sample(Sampler, GetTiledUV(Input.UV)).rgb;

	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);

	float3 normal = (NormalTexture.Sample(Sampler, GetTiledUV(Input.UV)) * 2.0f - 1.0f).xyz;
	normal = mul(normal, TBN);

	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

	float4 Final = 0;
	Final.rgb = CalculatePBRLighting(Input.PosW, normal, viewDir, albedo, roughness, metallic, ambientOcclusion, Input.ShadowPosH, Input.PosH.z);
	Final.a = GlobalOpacity;

	return Final;
}

#endif	// #if USE_DEFERRED_SHADING == 1
