//=============================================================================
// Skybox_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"

TextureCube SkyTexture;
SamplerState Sampler;

struct OUTPUT_VERTEX
{
	float4 PosH		: SV_POSITION;
	float3 PosL		: TEXCOORD0;
	float4 PosW		: TEXCOORD1;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 skyColor = MakeLinearColorFromGammaSpace(SkyTexture.Sample(Sampler, normalize(Input.PosL)));
	float4 sunColor = float4(0, 0, 0, 1);

#if 1
	if (DirectionalLightCount > 0)
	{
		float3 viewDir = normalize(CameraPos.xyz - Input.PosW.xyz);
		float3 lightDir = -DirectionalLight[0].Direction.xyz;
		float SpecularIntensity = max(pow(saturate(dot(viewDir, lightDir)), 128), 0.0f);
		sunColor.rgb = saturate(DirectionalLight[0].Color * SpecularIntensity);
	}
#endif

	return saturate(skyColor + sunColor);

}