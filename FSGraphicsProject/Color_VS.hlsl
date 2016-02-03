//=============================================================================
// Color_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
struct INPUT_VERTEX
{
	float4 PosL		: POSITION;
	float4 Color	: COLOR0;
};

struct OUTPUT_VERTEX
{
	float4 Color	: COLOR;
	float4 PosH		: SV_POSITION;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 worldMatrix;
};

cbuffer cbScene : register(b1)
{
	float4x4 viewProjMatrix;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	Out.PosH = mul(worldMatrix, Input.PosL);
	Out.PosH = mul(viewProjMatrix, Out.PosH);
	Out.Color = Input.Color;

	return Out;
}