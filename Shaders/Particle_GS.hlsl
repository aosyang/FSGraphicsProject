//=============================================================================
// Particle_GS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferVS.h"

struct OUTPUT_VERTEX
{
	float4 Color			: COLOR;
	float4 PosH				: SV_POSITION;
	float4 PosW				: POSITION;
	float2 SizeRot			: TEXCOORD0;
	float4 UVScaleOffset	: TEXCOORD1;
};

struct GSOutput
{
	float4 Color		: COLOR;
	float2 UV			: TEXCOORD0;
	float3 PosW			: TEXCOORD1;
	float4 ShadowPosH[3]	: TEXCOORD2;
	float3 NormalW		: NORMAL;
	float3 TangentW		: TANGENT;
	float4 PosH			: SV_POSITION;
};

[maxvertexcount(4)]
void main(
	point OUTPUT_VERTEX Input[1],
	inout TriangleStream< GSOutput > output
)
{
	float3x3 invView = transpose((float3x3)viewMatrix);

	//float3 right = normalize(float3(invView._m00, invView._m01, invView._m02));
	//float3 forward = normalize(Input[0].PosW.xyz - cameraPos.xyz);
	//float3 up = normalize(cross(forward, right));

	float3 up = float3(0, 1, 0);
	float3 forward = normalize(Input[0].PosW.xyz - cameraPos.xyz);
	float3 right = normalize(cross(up, forward));
	up = normalize(cross(forward, right));

	float theta = Input[0].SizeRot.y;
	float3x3 rot = float3x3(cos(theta), -sin(theta), 0,
							sin(theta), cos(theta), 0,
							0, 0, 1);

	invView = mul(rot, float3x3(right, up, forward));

	float4 offset[4];
	offset[0] = float4(mul(float3(-0.5f, -0.5f, 0.0f), invView), 0.0f);
	offset[1] = float4(mul(float3(-0.5f, 0.5f, 0.0f), invView), 0.0f);
	offset[2] = float4(mul(float3(0.5f, -0.5f, 0.0f), invView), 0.0f);
	offset[3] = float4(mul(float3(0.5f, 0.5f, 0.0f), invView), 0.0f);

	float3 normal = mul(float3(0.0f, 0.0f, -1.0f), invView);
	float3 tangent = mul(float3(1.0f, 0.0f, 0.0f), invView);

	float2 uv[4];
	uv[0] = float2(0.0f, 1.0f);
	uv[1] = float2(0.0f, 0.0f);
	uv[2] = float2(1.0f, 1.0f);
	uv[3] = float2(1.0f, 0.0f);

	GSOutput Vert[4];
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		Vert[i].PosW = (Input[0].PosW + offset[i] * Input[0].SizeRot.x).xyz;
		Vert[i].PosH = mul(float4(Vert[i].PosW, 1.0f), viewProjMatrix);
		Vert[i].ShadowPosH[0] = mul(float4(Vert[i].PosW, 1.0f), shadowViewProjBiasedMatrix[0]);
		Vert[i].ShadowPosH[1] = mul(float4(Vert[i].PosW, 1.0f), shadowViewProjBiasedMatrix[1]);
		Vert[i].ShadowPosH[2] = mul(float4(Vert[i].PosW, 1.0f), shadowViewProjBiasedMatrix[2]);
		Vert[i].Color = Input[0].Color;
		Vert[i].UV = uv[i] * Input[0].UVScaleOffset.xy + Input[0].UVScaleOffset.zw;
		Vert[i].NormalW = normal;
		Vert[i].TangentW = tangent;

		output.Append(Vert[i]);
	}
}