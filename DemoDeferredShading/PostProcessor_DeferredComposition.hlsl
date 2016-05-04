//=============================================================================
// PostProcessor_DeferredComposition.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "../Shaders/ConstBufferPS.h"

Texture2D AlbedoTexture		: register(t0);
Texture2D PositionTexture	: register(t1);
Texture2D NormalTexture		: register(t2);

SamplerState Sampler;

float3 CalculateAmbientLight(float3 normal, float4 highHemisphereColor, float4 lowHemisphereColor)
{
	return saturate(dot(normal, float3(0.0f, 1.0f, 0.0f)) / 2.0f + 0.5f) * highHemisphereColor.rgb * highHemisphereColor.w +
		   saturate(dot(normal, float3(0.0f, -1.0f, 0.0f)) / 2.0f + 0.5f) * lowHemisphereColor.rgb * lowHemisphereColor.w;
}


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
	float4 Final = (float4)1;
	Final.rgb = Albedo.rgb * CalculateAmbientLight(Normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);

	return Final;
}