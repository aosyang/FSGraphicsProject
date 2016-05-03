//=============================================================================
// Font_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
	float4 PosL		: POSITION;
	float4 Color	: COLOR0;
	float2 UV		: TEXCOORD0;
};

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float4 Color	: COLOR;
	float2 UV		: TEXCOORD0;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	Out.PosH = (float4)0;
	Out.PosH.x = Input.PosL.x * 2 / ScreenSize.x - 1;
	Out.PosH.y = -(Input.PosL.y * 2 / ScreenSize.y - 1);
	Out.PosH.w = 1.0f;
	Out.UV = Input.UV;
	Out.Color = Input.Color;

	return Out;
}