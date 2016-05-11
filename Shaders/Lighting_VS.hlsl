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
#if USE_SKINNING == 1
	int4   BoneId	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;
#endif
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

OUTPUT_VERTEX main(INPUT_VERTEX Input
#if USE_INSTANCING == 1
				 , uint InstID : SV_InstanceID
#endif
	)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	float4 worldPos = (float4)0;
	float3 Normal = (float3)0;

#if USE_SKINNING == 1
	for (int i = 0; i < 4; i++)
	{
		worldPos += mul(float4(Input.PosL, 1.0f), boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
		Normal += mul(Input.Normal, (float3x3)boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
	}
	Normal = normalize(Normal);
#elif USE_INSTANCING == 1
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

	Out.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	Out.NormalW = Normal;

	Out.UV = Input.UV;

	return Out;
}