//=============================================================================
// SkinnedDefault_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
	float2 UV0		: TEXCOORD0;
	float3 Normal	: NORMAL;
	float2 UV1		: TEXCOORD1;
	int4   BoneId	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;
};

struct OUTPUT_VERTEX
{
	float4 VColor	: COLOR;
	float4 PosH		: SV_POSITION;
	float2 UV		: TEXCOORD0;
};


OUTPUT_VERTEX main( INPUT_VERTEX Input )
{
	OUTPUT_VERTEX Output = (OUTPUT_VERTEX)0;

	float3 Normal = (float3)0;

	for (int i = 0; i < 4; i++)
	{
		Output.PosH += mul(float4(Input.PosL, 1.0f), boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
		Normal += mul(Input.Normal, (float3x3)boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
	}

	//Output.PosH = mul(float4(Input.PosL, 1.0f), worldMatrix);
	//Normal = mul(Input.Normal, (float3x3)worldMatrix);

	Output.PosH = mul(Output.PosH, viewProjMatrix);

	Output.UV = Input.UV0;

	Output.VColor.rgb = saturate(dot(Normal, float3(0, 1, 0)) / 2 + 0.5f);
	Output.VColor.a = 1.0f;

	return Output;
}