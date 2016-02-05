//=============================================================================
// Skybox_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "SharedDefines.h"

struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
};

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float3 PosW		: TEXCOORD0;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	float4 pos = (float4)1;
	pos.xyz = Input.PosL * 100.0f + cameraPos.xyz;

	Out.PosH = mul(pos, worldMatrix);
	Out.PosH = mul(Out.PosH, viewProjMatrix);

	Out.PosW = Input.PosL.xyz;

	return Out;
}