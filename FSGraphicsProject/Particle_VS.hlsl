//=============================================================================
// Particle_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
	float4 PosL		: POSITION;
	float4 Color	: COLOR0;
	float  Rotation : TEXCOORD0;
};

struct OUTPUT_VERTEX
{
	float4 Color	: COLOR;
	float4 PosH		: SV_POSITION;
	float4 PosW		: POSITION;
	float2 SizeRot	: TEXCOORD0;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;
	
	Out.PosW = mul(float4(Input.PosL.xyz, 1.0f), worldMatrix);
	Out.PosH = mul(Out.PosW, viewProjMatrix);
	Out.Color = Input.Color;
	Out.SizeRot.x = Input.PosL.w;
	Out.SizeRot.y = Input.Rotation;

	return Out;
}