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
#if USE_SKINNING == 1
	int4   BoneId	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;
#endif
};

struct OUTPUT_VERTEX
{
	float4 Color	: COLOR;
	float4 PosH		: SV_POSITION;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	float3 Normal = (float3)0;

#if USE_SKINNING == 1
	for (int i = 0; i < 4; i++)
	{
		Out.PosH += mul(float4(Input.PosL, 1.0f), boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
		Normal += mul(Input.Normal, (float3x3)boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
	}
	Normal = normalize(Normal);
#else
	Out.PosH = mul(float4(Input.PosL, 1.0f), worldMatrix);

	float3 Normal = mul(Input.Normal, (float3x3)worldMatrix);
#endif

	Out.PosH = mul(Out.PosH, viewProjMatrix);

	Out.Color.rgb = saturate(dot(Normal, float3(0, 1, 0)) / 2 + 0.5f);
	Out.Color.a = 1.0f;

	return Out;
}