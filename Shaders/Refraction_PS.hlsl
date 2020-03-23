//=============================================================================
// Refraction_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightingModel_Phong.hlsli"

Texture2D ScreenTexture	: register(t0);

struct OUTPUT_VERTEX
{
	float4 PosH				: SV_POSITION;
	float4 UV				: TEXCOORD0;
	float3 NormalH			: TEXCOORD1;
	float3 PosW				: TEXCOORD2;
	float4 ShadowPosH[3]	: TEXCOORD3;
	float3 NormalW			: NORMAL;
};

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

	float2 uvOffset = float2(Input.NormalH.x / ScreenSize.x, Input.NormalH.y / ScreenSize.y) * 64.0f;
	float3 distortion = ScreenTexture.Sample(Sampler, Input.UV.xy / Input.UV.z * 0.5f + 0.5f + uvOffset).rgb;
	float3 BaseColor = float3(0.75f, 1.0f, 0.8f);
	float ambientRim = 1.0f - saturate(dot(Input.NormalH, float3(0, 0, -1)));

	// Compose the final color
	float4 Final;
	Final.rgb = lerp(distortion, Ambient, ambientRim)* BaseColor + LightColor.Specular;
	Final.a = GlobalOpacity;
	return Final;
}
