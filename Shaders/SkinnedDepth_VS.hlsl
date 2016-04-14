//=============================================================================
// SkinnedDepth_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
	float2 UV0		: TEXCOORD0;
	float3 Normal	: NORMAL;
	int4   BoneId	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;
};

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
};


OUTPUT_VERTEX main( INPUT_VERTEX Input )
{
	OUTPUT_VERTEX Output = (OUTPUT_VERTEX)0;

	for (int i = 0; i < 4; i++)
	{
		Output.PosH += mul(float4(Input.PosL, 1.0f), boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
	}

	Output.PosH = mul(Output.PosH, shadowViewProjMatrix);

	return Output;
}