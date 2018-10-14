//=============================================================================
// MeshVertexSignature.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
	float3 Normal	: NORMAL;
	float3 Tangent	: TANGENT;
	float2 UV0		: TEXCOORD0;
	float2 UV1		: TEXCOORD1;
};

float4 main(INPUT_VERTEX Input) : SV_POSITION
{
	return float4(0, 0, 0, 0);
}