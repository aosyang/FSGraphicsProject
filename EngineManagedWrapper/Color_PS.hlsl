//=============================================================================
// Color_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

struct OUTPUT_VERTEX
{
	float4 Color : COLOR;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	return Input.Color;
}
