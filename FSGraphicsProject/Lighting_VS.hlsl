//=============================================================================
// Lighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
	float2 UV		: TEXCOORD0;
	float3 Normal	: NORMAL;
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

	Out.PosH = mul(worldMatrix, float4(Input.PosL, 1.0f));
	Out.PosH = mul(viewProjMatrix, Out.PosH);
	Out.Color = float4(Input.Normal, 1.0f);

	return Out;
}