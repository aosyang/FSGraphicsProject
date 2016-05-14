//=============================================================================
// Color_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"

struct OUTPUT_VERTEX
{
	float4 Color : COLOR;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Final = MakeLinearColorFromGammaSpace(Input.Color);
	Final.a *= GlobalOpacity;
	return Final;
}
