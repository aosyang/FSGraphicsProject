//=============================================================================
// Default_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float4 Color	: COLOR;
	float3 NormalW	: NORMAL;
	float4 PosW		: TEXCOORD0;
	float3 NormalV	: TEXCOORD1;
};

#if USE_DEFERRED_SHADING == 1
struct OUTPUT_PIXEL
{
	float4 Albedo		: SV_Target0;
	float4 WorldPos		: SV_Target1;
	float  Depth		: SV_Target2;
	float4 NormalW		: SV_Target3;
	float4 NormalV		: SV_Target4;
};

OUTPUT_PIXEL main(OUTPUT_VERTEX Input) : SV_TARGET
{
	OUTPUT_PIXEL Out = (OUTPUT_PIXEL)0;
	Out.Albedo = float4(1, 1, 1, 1);
	Out.WorldPos = Input.PosW;
	Out.Depth = LinearizeDepth(Input.PosH.z);
	Out.NormalW = float4(normalize(Input.NormalW), 1);
	Out.NormalV = float4(normalize(Input.NormalV), 1);

	return Out;
}

#else

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	return Input.Color;
}

#endif