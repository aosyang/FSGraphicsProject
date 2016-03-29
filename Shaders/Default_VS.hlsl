//=============================================================================
// Default_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
	float2 UV		: TEXCOORD0;
	float3 Normal	: NORMAL;
};

struct OUTPUT_VERTEX
{
	float4 Color	: COLOR;
	float4 PosH		: SV_POSITION;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	Out.PosH = mul(float4(Input.PosL, 1.0f), worldMatrix);
	Out.PosH = mul(Out.PosH, viewProjMatrix);

	float3 Normal = mul(Input.Normal, (float3x3)worldMatrix);
	Out.Color.rgb = saturate(dot(Normal, float3(0, 1, 0)) / 2 + 0.5f);
	Out.Color.a = 1.0f;

	return Out;
}