//=============================================================================
// InstancedDepth_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
};

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input, uint InstID : SV_InstanceID)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	Out.PosH = mul(float4(Input.PosL, 1.0f), instancedWorldMatrix[InstID]);
	Out.PosH = mul(Out.PosH, shadowViewProjMatrix);

	return Out;
}
