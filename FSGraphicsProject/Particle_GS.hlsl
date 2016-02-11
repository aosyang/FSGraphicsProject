//=============================================================================
// Particle_GS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct OUTPUT_VERTEX
{
	float4 Color	: COLOR;
	float4 PosH		: SV_POSITION;
	float4 PosW		: POSITION;
	float Size		: TEXCOORD0;
};

struct GSOutput
{
	float4 Color	: COLOR;
	float4 PosH		: SV_POSITION;
};

[maxvertexcount(6)]
void main(
	point OUTPUT_VERTEX Input[1],
	inout TriangleStream< GSOutput > output
)
{
	float3x3 invView = transpose((float3x3)viewMatrix);
	float4 offset[4];
	offset[0] = float4(mul(float3(-0.5f, -0.5f, 0.0f), invView), 0.0f);
	offset[1] = float4(mul(float3(-0.5f, 0.5f, 0.0f), invView), 0.0f);
	offset[2] = float4(mul(float3(0.5f, 0.5f, 0.0f), invView), 0.0f);
	offset[3] = float4(mul(float3(0.5f, -0.5f, 0.0f), invView), 0.0f);

	GSOutput Vert[4];
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		Vert[i].PosH = mul(Input[0].PosW + offset[i] * Input[0].Size, viewProjMatrix);
		Vert[i].Color = Input[0].Color;
	}

	output.Append(Vert[0]);
	output.Append(Vert[1]);
	output.Append(Vert[2]);
	output.RestartStrip();

	output.Append(Vert[0]);
	output.Append(Vert[2]);
	output.Append(Vert[3]);
	output.RestartStrip();
}