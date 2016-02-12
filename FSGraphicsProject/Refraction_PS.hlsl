//=============================================================================
// Refraction_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

Texture2D ScreenTexture	: register(t0);
SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH			: SV_POSITION;
	float4 UV			: TEXCOORD0;
	float3 NormalH		: TEXCOORD1;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float2 uvOffset = float2(Input.NormalH.x / 1024.0f, Input.NormalH.y / 768.0f) * 64.0f;
	return ScreenTexture.Sample(Sampler, Input.UV.xy / Input.UV.w * 0.5f + 0.5f + uvOffset) * float4(0.5f, 1.0f, 0.75f, 1.0f);
}