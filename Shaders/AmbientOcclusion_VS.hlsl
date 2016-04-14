//=============================================================================
// AmbientOcclusion_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
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
};

struct OUTPUT_VERTEX
{
	float4 Color		: COLOR;
	float4 PosH			: SV_POSITION;
	float2 UV0			: TEXCOORD0;
	float2 UV1			: TEXCOORD1;
	float3 PosW			: TEXCOORD2;
	float4 ShadowPosH	: TEXCOORD3;
	float3 NormalW		: NORMAL;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	float4 worldPos = mul(float4(Input.PosL, 1.0f), worldMatrix);
	Out.PosW = worldPos.xyz;
	Out.PosH = mul(worldPos, viewProjMatrix);
	Out.ShadowPosH = mul(worldPos, shadowViewProjBiasedMatrix);

	Out.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	Out.NormalW = mul(Input.Normal, (float3x3)worldMatrix);

	Out.UV0 = Input.UV0;
	Out.UV1 = Input.UV1;

	return Out;
}