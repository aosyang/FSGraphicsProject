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
	float2 UV		: TEXCOORD0;
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

	float3 Normal = mul((float3x3)worldMatrix, Input.Normal);

	Out.PosH = mul(worldMatrix, float4(Input.PosL, 1.0f));
	Out.PosH = mul(viewProjMatrix, Out.PosH);

	float3 lightVec = float3(0.25f, 1.0f, 0.5f);
	lightVec = normalize(lightVec);
	float i = dot(Normal, lightVec);
	Out.Color = float4(i, i, i, 1.0f);
	
	//Out.Color.rgb = Normal / 2.0f + float3(0.5f, 0.5f, 0.5f);
	//Out.Color.a = 1.0f;

	Out.UV = Input.UV;

	return Out;
}