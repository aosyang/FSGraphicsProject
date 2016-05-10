//=============================================================================
// Depth_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
#if USE_SKINNING == 1
	float2 UV0		: TEXCOORD0;		// Need this for vertex input layout
	float3 Normal	: NORMAL;			// Need this for vertex input layout
	int4   BoneId	: BLENDINDICES;
	float4 Weight	: BLENDWEIGHT;
#endif
};

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

#if USE_SKINNING == 1
	for (int i = 0; i < 4; i++)
	{
		Out.PosH += mul(float4(Input.PosL, 1.0f), boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
	}
#else
	Out.PosH = mul(float4(Input.PosL, 1.0f), worldMatrix);
#endif

	Out.PosH = mul(Out.PosH, shadowViewProjMatrix[cascadedShadowIndex]);

	return Out;
}
