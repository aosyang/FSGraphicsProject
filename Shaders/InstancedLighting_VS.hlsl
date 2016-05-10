//=============================================================================
// InstancedLighting_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
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
	float4 Color			: COLOR;
	float4 PosH				: SV_POSITION;
	float2 UV				: TEXCOORD0;
	float3 NormalW			: TEXCOORD1;
	float3 PosW				: TEXCOORD2;
	float4 ShadowPosH[3]	: TEXCOORD3;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input, uint InstID : SV_InstanceID)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	float4 worldPos = mul(float4(Input.PosL, 1.0f), instancedWorldMatrix[InstID]);
	Out.PosW = worldPos.xyz;
	Out.PosH = mul(worldPos, viewProjMatrix);
	Out.ShadowPosH[0] = mul(worldPos, shadowViewProjBiasedMatrix[0]);
	Out.ShadowPosH[1] = mul(worldPos, shadowViewProjBiasedMatrix[1]);
	Out.ShadowPosH[2] = mul(worldPos, shadowViewProjBiasedMatrix[2]);

	Out.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	Out.NormalW = mul(Input.Normal, (float3x3)instancedWorldMatrix[InstID]);

	Out.UV = Input.UV;

	return Out;
}