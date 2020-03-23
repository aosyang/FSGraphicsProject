//=============================================================================
// Lighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightingModel_Phong.hlsli"

Texture2D DiffuseTexture	: register(t0);

struct OUTPUT_VERTEX
{
	float4 Color			: COLOR;
	float4 PosH				: SV_POSITION;		// Vertex position in homogeneous coordinates
	float2 UV				: TEXCOORD0;		// Texture coordinates
	float3 NormalW			: TEXCOORD1;		// Normal direction in world space
	float3 PosW				: TEXCOORD2;		// Vertex position in world space
	float3 NormalV			: TEXCOORD3;		// Normal direction in view space
	float4 ShadowPosH[3]	: TEXCOORD4;
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
	Out.Albedo = DiffuseTexture.Sample(Sampler, Input.UV);
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
	float3 Ambient = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);

	// Color from diffuse texture
	float4 DiffuseSample = DiffuseTexture.Sample(Sampler, Input.UV);

	// Compose the final color
	float4 Final;
	Final.rgb = (Ambient + LightColor.Diffuse) * DiffuseSample.rgb + LightColor.Specular;
	Final.a = DiffuseSample.a * GlobalOpacity;
	return Final;
}

#endif	// #if USE_DEFERRED_SHADING == 1