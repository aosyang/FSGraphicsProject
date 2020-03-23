//=============================================================================
// BumpSpecularLighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightingModel_Phong.hlsli"

Texture2D DiffuseTexture	: register(t0);
Texture2D NormalTexture		: register(t1);
Texture2D SpecularTexture	: register(t2);


struct OUTPUT_VERTEX
{
	float4 Color			: COLOR;
	float4 PosH				: SV_POSITION;
	float2 UV				: TEXCOORD0;
	float3 PosW				: TEXCOORD1;
	float3 NormalW			: NORMAL;
	float3 TangentW			: TANGENT;
	float4 ShadowPosH[3]	: TEXCOORD3;
};

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);

	float3 normal = (NormalTexture.Sample(Sampler, Input.UV) * 2.0f - 1.0f).xyz;
	normal = mul(normal, TBN);

	float4 specularColorAndPower = SpecularTexture.Sample(Sampler, Input.UV);
	specularColorAndPower.w = pow(2.0, 13.0 * specularColorAndPower.w);

	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

	PhongLightingData LightColor = CalculatePhongLighting(Input.PosW, normal, Input.ShadowPosH, Input.PosH.z, viewDir, specularColorAndPower);

	// Hemisphere ambient color
	float3 Ambient = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);

	// Color from diffuse texture
	float4 DiffuseSample = DiffuseTexture.Sample(Sampler, Input.UV);

	// Compose the final color
	float4 Final;
	Final.rgb = (Ambient + LightColor.Diffuse) * DiffuseSample.rgb + LightColor.Specular;
	Final.a = DiffuseSample.a * GlobalOpacity;
	return Final;
}
