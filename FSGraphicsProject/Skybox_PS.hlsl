//=============================================================================
// Skybox_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
TextureCube SkyTexture;
SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float3 PosW		: TEXCOORD0;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	return SkyTexture.Sample(Sampler, normalize(Input.PosW));
}