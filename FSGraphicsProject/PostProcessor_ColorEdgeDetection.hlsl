//=============================================================================
// PostProcessor_ColorEdgeDetection.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

Texture2D ScreenTexture : register(t0);

SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float2 UV		: TEXCOORD0;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Final = (float4)0;
	Final = saturate(ScreenTexture.Sample(Sampler, Input.UV) - ScreenTexture.Sample(Sampler, Input.UV + float2(1.0f / ScreenSize.x, 0))) +
			saturate(ScreenTexture.Sample(Sampler, Input.UV) - ScreenTexture.Sample(Sampler, Input.UV - float2(1.0f / ScreenSize.x, 0))) +
			saturate(ScreenTexture.Sample(Sampler, Input.UV) - ScreenTexture.Sample(Sampler, Input.UV + float2(0, 1.0f / ScreenSize.y))) +
			saturate(ScreenTexture.Sample(Sampler, Input.UV) - ScreenTexture.Sample(Sampler, Input.UV - float2(0, 1.0f / ScreenSize.y)));
	Final = saturate(Final);

	Final.a = 1.0f;
	return Final;
}
