//=============================================================================
// Lighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

Texture2D DiffuseTexture;
SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 Color	: COLOR;
	float4 PosH		: SV_POSITION;
	float2 UV		: TEXCOORD0;
	float3 NormalW	: TEXCOORD1;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 lightColor = (float4)0;
	for (int i = 0; i < DirectionalLightCount; i++)
	{
		float3 normal = normalize(Input.NormalW);
		float intensity = dot(normal, DirectionalLight[i].Direction.xyz);
		lightColor.rgb += saturate(intensity) * DirectionalLight[i].Color.rgb;
	}

	lightColor.rgb = saturate(lightColor.rgb);
	lightColor.a = 1.0f;

	return lightColor * DiffuseTexture.Sample(Sampler, Input.UV);
}
