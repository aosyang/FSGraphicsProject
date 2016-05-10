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
	float4 Color			: COLOR;
	float4 PosH				: SV_POSITION;
	float2 UV0				: TEXCOORD0;
	float2 UV1				: TEXCOORD1;
	float3 PosW				: TEXCOORD2;
	float4 ShadowPosH[3]	: TEXCOORD3;
	float3 NormalW			: NORMAL;
};

#define DEBUG_CASCADED_LEVELS 0

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Diffuse = (float4)0;
	float4 Specular = (float4)0;
	float4 Ambient = (float4)0;
	float3 normal = normalize(Input.NormalW);
	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

	Input.ShadowPosH[0].xyz /= Input.ShadowPosH[0].w;
	Input.ShadowPosH[1].xyz /= Input.ShadowPosH[1].w;
	Input.ShadowPosH[2].xyz /= Input.ShadowPosH[2].w;

	for (int id = 0; id < DirectionalLightCount; id++)
	{
		float shadowOffset[3] =
		{
			0.01f * saturate(1 - dot(DirectionalLight[id].Direction.xyz, normal)),
			0.01f * saturate(1 - dot(DirectionalLight[id].Direction.xyz, normal)),
			0.01 + 0.01f * saturate(1 - dot(DirectionalLight[id].Direction.xyz, normal)),
		};

		float lit = 1.0f;

		if (id == 0)
		{
			lit = SampleCascadedShadowMap(Input.ShadowPosH, dot(DirectionalLight[id].Direction.xyz, normal));
		}

		// Diffuse lighting
		Diffuse.rgb += lit * CalculateDiffuseLight(normal, DirectionalLight[id].Direction.xyz, DirectionalLight[id].Color);

		// Specular lighting
		Specular.rgb += lit * CalculateSpecularLight(normal, DirectionalLight[id].Direction.xyz, viewDir, DirectionalLight[id].Color, SpecularColorAndPower);
	}

	for (int ip = 0; ip < PointLightCount; ip++)
	{
		float3 lightVec = PointLight[ip].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float attenuation = 1.0f - saturate(length(lightVec) / PointLight[ip].PosAndRadius.w);

		// Diffuse lighting
		Diffuse.rgb += attenuation * CalculateDiffuseLight(normal, lightDir, PointLight[ip].Color);

		// Specular lighting
		Specular.rgb += attenuation * CalculateSpecularLight(normal, lightDir, viewDir, PointLight[ip].Color, SpecularColorAndPower);
	}

	for (int is = 0; is < SpotlightCount; is++)
	{
		float3 lightVec = Spotlight[is].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[is].Direction.xyz));
		float radiusAtt = 1.0f - saturate(length(lightVec) / Spotlight[is].PosAndRadius.w);
		float coneAtt = 1.0f - saturate((Spotlight[is].ConeRatio.x - surfaceRatio) / (Spotlight[is].ConeRatio.x - Spotlight[is].ConeRatio.y));
		float attenuation = radiusAtt * coneAtt;

		// Diffuse lighting
		Diffuse.rgb += attenuation * CalculateDiffuseLight(normal, lightDir, Spotlight[is].Color);

		// Specular lighting
		Specular.rgb += attenuation * CalculateSpecularLight(normal, lightDir, viewDir, Spotlight[is].Color, SpecularColorAndPower);
	}

	Diffuse.a = 1.0f;

	Ambient.rgb = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor) *
				  AmbientOcclusionTexture.Sample(Sampler, Input.UV1).rgb;

	float4 Final = (Ambient + Diffuse) * MakeLinearColorFromGammaSpace(DiffuseTexture.Sample(Sampler, Input.UV0)) + Specular;
	Final.a *= GlobalOpacity;

#if DEBUG_CASCADED_LEVELS == 1
	if (Input.ShadowPosH[0].x > 0.01 && Input.ShadowPosH[0].x < 0.99 &&
		Input.ShadowPosH[0].y > 0.01 && Input.ShadowPosH[0].y < 0.99 &&
		Input.ShadowPosH[0].z > 0.01 && Input.ShadowPosH[0].z < 0.99)
	{
		Final *= float4(1, 0, 0, 1);
	}
	else if (Input.ShadowPosH[1].x > 0.01 && Input.ShadowPosH[1].x < 0.99 &&
		Input.ShadowPosH[1].y > 0.01 && Input.ShadowPosH[1].y < 0.99 &&
		Input.ShadowPosH[1].z > 0.01 && Input.ShadowPosH[1].z < 0.99)
	{
		Final *= float4(0, 1, 0, 1);
	}
	else if (Input.ShadowPosH[2].x > 0 && Input.ShadowPosH[2].x < 1 &&
		Input.ShadowPosH[2].y > 0 && Input.ShadowPosH[2].y < 1 &&
		Input.ShadowPosH[2].z > 0 && Input.ShadowPosH[2].z < 1)
	{
		Final *= float4(0, 0, 1, 1);
	}
#endif


	return Final;
}
