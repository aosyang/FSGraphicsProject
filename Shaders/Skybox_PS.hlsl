//=============================================================================
// Skybox_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

TextureCube SkyTexture;
SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float3 PosL		: TEXCOORD0;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	return MakeLinearColorFromGammaSpace(SkyTexture.Sample(Sampler, normalize(Input.PosL)));
}