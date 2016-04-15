//=============================================================================
// Lighting_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
	float2 UV		: TEXCOORD0;
	float3 Normal	: NORMAL;
	int4   BoneId	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;
};

struct OUTPUT_VERTEX
{
	float4 Color		: COLOR;
	float4 PosH			: SV_POSITION;
	float2 UV			: TEXCOORD0;
	float3 NormalW		: TEXCOORD1;
	float3 PosW			: TEXCOORD2;
	float4 ShadowPosH	: TEXCOORD3;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	float4 worldPos = (float4)0;
	float3 Normal = (float3)0;

	for (int i = 0; i < 4; i++)
	{
		worldPos += mul(float4(Input.PosL, 1.0f), boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
		Normal += mul(Input.Normal, (float3x3)boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
	}

	Out.PosW = worldPos.xyz;
	Out.PosH = mul(worldPos, viewProjMatrix);
	Out.ShadowPosH = mul(worldPos, shadowViewProjBiasedMatrix);

	Out.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	Out.NormalW = Normal;

	Out.UV = Input.UV;

	return Out;
}