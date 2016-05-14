//=============================================================================
// PostProcessor_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
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
	float2 UV		: TEXCOORD0;
	float3 ViewRay	: TEXCOORD1;			// For screen space reflection
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	Out.PosH.xy = sign(Input.PosL.xy);
	Out.PosH.zw = float2(0.5f, 1.0f);
	Out.UV.x = 0.5f + Out.PosH.x * 0.5f;
	Out.UV.y = 0.5f - Out.PosH.y * 0.5f;

	float4 viewRay = mul(float4(Out.PosH.xy, -1, 1), invProjMatrix);
	Out.ViewRay = viewRay.xyz / viewRay.w;

	return Out;
}
