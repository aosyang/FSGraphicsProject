//=============================================================================
// AmbientOcclusion_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightShaderCommon.hlsli"

Texture2D DiffuseTexture			: register(t0);
Texture2D AmbientOcclusionTexture	: register(t1);

struct OUTPUT_VERTEX
{
	float4 PosH			: SV_POSITION;
	float2 UV0			: TEXCOORD0;
	float2 UV1			: TEXCOORD1;
	float3 PosW			: TEXCOORD2;
	float3 NormalW		: NORMAL;
};

struct OUTPUT_PIXEL
{
	float4 Albedo		: SV_Target0;
	float4 WorldPos		: SV_Target1;
	float4 Normal		: SV_Target2;
};

OUTPUT_PIXEL main(OUTPUT_VERTEX Input) : SV_TARGET
{
	OUTPUT_PIXEL Out = (OUTPUT_PIXEL)0;
	Out.Albedo = MakeLinearColorFromGammaSpace(DiffuseTexture.Sample(Sampler, Input.UV0));
	Out.WorldPos = float4(Input.PosW, 1);
	Out.Normal = float4(normalize(Input.NormalW), 1);

	return Out;
}
