//=============================================================================
// Depth_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
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
};

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input
#if USE_INSTANCING == 1
				 , uint InstID : SV_InstanceID
#endif
	)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

#if USE_SKINNING == 1
	for (int i = 0; i < 4; i++)
	{
		Out.PosH += mul(float4(Input.PosL, 1.0f), boneMatrix[Input.BoneId[i]]) * Input.Weight[i];
	}
#elif USE_INSTANCING == 1
	Out.PosH = mul(float4(Input.PosL, 1.0f), instancedWorldMatrix[InstID]);
#else
	Out.PosH = mul(float4(Input.PosL, 1.0f), worldMatrix);
#endif

	Out.PosH = mul(Out.PosH, shadowViewProjMatrix[cascadedShadowIndex]);

	return Out;
}
