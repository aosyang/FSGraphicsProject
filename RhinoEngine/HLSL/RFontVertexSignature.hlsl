//=============================================================================
// RFontVertexSignature.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

struct INPUT_VERTEX
{
	float4 PosL		: POSITION;
	float4 Color	: COLOR0;
	float2 UV		: TEXCOORD0;
};

float4 main(INPUT_VERTEX Input) : SV_POSITION
{
	return float4(0, 0, 0, 0);
}