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
	float4 PosH				: SV_POSITION;
	float2 UV0				: TEXCOORD0;
	float2 UV1				: TEXCOORD1;
	float3 PosW				: TEXCOORD2;
	float3 NormalV			: TEXCOORD3;
	float4 ShadowPosH[3]	: TEXCOORD4;
	float3 NormalW			: NORMAL;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input
#if USE_INSTANCING == 1
				 , uint InstID : SV_InstanceID
#endif
	)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	float4 worldPos = (float4)0;
	float3 Normal = (float3)0;

#if USE_INSTANCING == 1
	worldPos = mul(float4(Input.PosL, 1.0f), instancedWorldMatrix[InstID]);
	Normal = mul(Input.Normal, (float3x3)instancedWorldMatrix[InstID]);
#else
	worldPos = mul(float4(Input.PosL, 1.0f), worldMatrix);
	Normal = mul(Input.Normal, (float3x3)worldMatrix);
#endif
	Out.PosW = worldPos.xyz;
	Out.PosH = mul(worldPos, viewProjMatrix);
	Out.ShadowPosH[0] = mul(worldPos, shadowViewProjBiasedMatrix[0]);
	Out.ShadowPosH[1] = mul(worldPos, shadowViewProjBiasedMatrix[1]);
	Out.ShadowPosH[2] = mul(worldPos, shadowViewProjBiasedMatrix[2]);

	Out.NormalW = Normal;
	Out.NormalV = mul(Normal, (float3x3)viewMatrix);

	Out.UV0 = Input.UV0;
	Out.UV1 = Input.UV1;

	return Out;
}