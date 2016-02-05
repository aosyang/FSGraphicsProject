//=============================================================================
// Skybox_VS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
struct INPUT_VERTEX
{
	float3 PosL		: POSITION;
};

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float3 PosW		: TEXCOORD0;
};

cbuffer cbPerObject : register(b0)
{
	float4x4 worldMatrix;
};

cbuffer cbScene : register(b1)
{
	float4x4	viewMatrix;
	float4x4	projMatrix;
	float4x4	viewProjMatrix;
	float4		cameraPos;
};

OUTPUT_VERTEX main(INPUT_VERTEX Input)
{
	OUTPUT_VERTEX Out = (OUTPUT_VERTEX)0;

	Out.PosH = mul(worldMatrix, float4(Input.PosL * 100.0f + cameraPos.xyz, 1.0f));
	Out.PosH.w = 1.0f;
	Out.PosH = mul(viewProjMatrix, Out.PosH);
	Out.PosW = Input.PosL.xyz;

	return Out;
}