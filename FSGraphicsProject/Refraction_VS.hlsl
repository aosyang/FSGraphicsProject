//=============================================================================
// Refraction_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
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
	float4 PosH			: SV_POSITION;
	float4 UV			: TEXCOORD0;
	float3 NormalH		: TEXCOORD1;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;
	
	Out.PosH = mul(float4(Input.PosL, 1.0f), worldMatrix);
	Out.PosH = mul(Out.PosH, viewProjMatrix);

	// SV_POSITION interpolation doesn't have perspective correction, use other semantics instead
	Out.UV = Out.PosH;
	Out.UV.y = 1.0f - Out.UV.y;

	Out.NormalH = mul(Input.Normal, (float3x3)worldMatrix);
	Out.NormalH = mul(Out.NormalH, (float3x3)viewProjMatrix);

	return Out;
}