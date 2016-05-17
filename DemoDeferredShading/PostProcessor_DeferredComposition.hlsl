//=============================================================================
// PostProcessor_DeferredComposition.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "../Shaders/ConstBufferPS.h"
#include "../Shaders/PixelShaderCommon.hlsli"
#include "../Shaders/LightShaderCommon.hlsli"

Texture2D AlbedoTexture		: register(t0);
Texture2D NormalTexture		: register(t2);

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float2 UV		: TEXCOORD0;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Albedo = AlbedoTexture.Sample(Sampler, Input.UV);
	float3 Normal = NormalTexture.Sample(Sampler, Input.UV).rgb;

	if (!any(Normal))
		return float4(Albedo.rgb, 1);

	float4 Final = (float4)1;
	Final.rgb = Albedo.rgb * Albedo.a * CalculateAmbientLight(Normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);

	return Final;
}