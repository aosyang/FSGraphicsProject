//=============================================================================
// Default_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
#if USE_SKINNING == 1
	int4   BoneId	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;
#endif
	float3 PosL		: POSITION;
	float2 UV		: TEXCOORD0;
	float3 Normal	: NORMAL;
};

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float4 Color	: COLOR;
	float3 NormalW	: NORMAL;
	float4 PosW		: TEXCOORD0;
	float3 NormalV	: TEXCOORD1;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	float3 Normal = (float3)0;

#if USE_SKINNING == 1
	for (int i = 0; i < 4; i++)
	{
		Out.PosW += mul(float4(Input.PosL, 1.0f), boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
		Normal += mul(Input.Normal, (float3x3)boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
	}
	Normal = normalize(Normal);
#else
	Out.PosW = mul(float4(Input.PosL, 1.0f), worldMatrix);

	Normal = mul(Input.Normal, (float3x3)worldMatrix);
#endif

	Out.PosH = mul(Out.PosW, viewProjMatrix);
	Out.NormalW = Normal;
	Out.NormalV = mul(Normal, viewMatrix);

	Out.Color.rgb = saturate(dot(Normal, float3(0, 1, 0)) / 2 + 0.5f);
	Out.Color.a = 1.0f;

	return Out;
}