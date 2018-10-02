//=============================================================================
// Lighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightShaderCommon.hlsli"

Texture2D DiffuseTexture	: register(t0);

struct OUTPUT_VERTEX
{
	float4 Color			: COLOR;
	float4 PosH				: SV_POSITION;
	float2 UV				: TEXCOORD0;
	float3 NormalW			: TEXCOORD1;
	float3 PosW				: TEXCOORD2;
	float3 NormalV			: TEXCOORD3;
	float4 ShadowPosH[3]	: TEXCOORD4;
};

#if USE_DEFERRED_SHADING == 1

struct OUTPUT_PIXEL
{
	float4 Albedo		: SV_Target0;
	float4 WorldPos		: SV_Target1;
	float4 NormalW		: SV_Target2;
	float4 NormalV		: SV_Target3;
};

OUTPUT_PIXEL main(OUTPUT_VERTEX Input) : SV_TARGET
{
	OUTPUT_PIXEL Out = (OUTPUT_PIXEL)0;
	Out.Albedo = DiffuseTexture.Sample(Sampler, Input.UV);
	Out.WorldPos = float4(Input.PosW, Input.PosH.z);
	Out.NormalW = float4(normalize(Input.NormalW), 1);
	Out.NormalV = float4(normalize(Input.NormalV), 1);

	return Out;
}

#else

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
		float lit = 1.0f;

		if (id == 0)
		{
			lit = SampleCascadedShadowMap(Input.ShadowPosH, dot(DirectionalLight[id].Direction.xyz, normal), Input.PosH.z);
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

	Ambient.rgb = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);

	float4 Final = (Ambient + Diffuse) * DiffuseTexture.Sample(Sampler, Input.UV) + Specular;
	Final.a *= GlobalOpacity;
	return Final;
}

#endif	// #if USE_DEFERRED_SHADING == 1