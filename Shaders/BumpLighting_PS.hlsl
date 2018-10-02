//=============================================================================
// BumpLighting_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightShaderCommon.hlsli"

Texture2D DiffuseTexture	: register(t0);
Texture2D NormalTexture		: register(t1);


struct OUTPUT_VERTEX
{
	float4 Color			: COLOR;
	float4 PosH				: SV_POSITION;
	float2 UV				: TEXCOORD0;
	float3 PosW				: TEXCOORD1;
	float3 NormalW			: NORMAL;
	float3 TangentW			: TANGENT;
	float4 ShadowPosH[3]	: TEXCOORD2;
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

	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);
	float3 normal = normalize((NormalTexture.Sample(Sampler, Input.UV) * 2.0f - 1.0f).xyz);
	float3 worldNormal = mul(normal, TBN);

	Out.NormalW = float4(worldNormal, 1);
	Out.NormalV = float4(mul(worldNormal, (float3x3)viewMatrix), 1);

	return Out;
}

#else

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Diffuse = (float4)0;
	float4 Specular = (float4)0;
	float4 Ambient = (float4)0;

	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);

	float3 normal = (NormalTexture.Sample(Sampler, Input.UV) * 2.0f - 1.0f).xyz;
	normal = mul(normal, TBN);

	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);

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
