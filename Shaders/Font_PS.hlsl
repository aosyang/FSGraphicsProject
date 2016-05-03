//=============================================================================
// Font_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

Texture2D FontTexture	: register(t0);
SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float4 Color	: COLOR;
	float2 UV		: TEXCOORD0;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float tex = FontTexture.Sample(Sampler, Input.UV).r;
	float4 Final = Input.Color * tex;
	Final = MakeLinearColorFromGammaSpace(Final);
	Final.a *= tex * GlobalOpacity;
	return Final;
}
