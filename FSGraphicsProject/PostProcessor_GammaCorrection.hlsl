//=============================================================================
// PostProcessor_GammaCorrection.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

Texture2D ScreenTexture : register(t0);

SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float2 UV		: TEXCOORD0;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Final = ScreenTexture.Sample(Sampler, Input.UV);
	Final.rgb = pow(Final.rgb, 1/2.2f);
	Final.a = 1.0f;
	return Final;
}