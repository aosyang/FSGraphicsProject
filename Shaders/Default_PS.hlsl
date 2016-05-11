//=============================================================================
// Default_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

struct OUTPUT_VERTEX
{
	float4 Color	: COLOR;
	float3 NormalW	: NORMAL;
	float4 PosW		: TEXCOORD0;
};

#if USE_DEFERRED_SHADING == 1
struct OUTPUT_PIXEL
{
	float4 Albedo		: SV_Target0;
	float4 WorldPos		: SV_Target1;
	float4 Normal		: SV_Target2;
};

OUTPUT_PIXEL main(OUTPUT_VERTEX Input) : SV_TARGET
{
	OUTPUT_PIXEL Out = (OUTPUT_PIXEL)0;
	Out.Albedo = Input.Color;
	Out.WorldPos = Input.PosW;
	Out.Normal = float4(normalize(Input.NormalW), 1);

	return Out;
}

#else

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	return Input.Color;
}

#endif