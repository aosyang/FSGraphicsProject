//=============================================================================
// BumpLightingPBR_PS.hlsl by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"
#include "LightShaderCommon.hlsli"
#include "BRDF.hlsli"

Texture2D DiffuseTexture		: register(t0);
Texture2D NormalTexture			: register(t1);
Texture2D RoughnessMetallicAO	: register(t2);
TextureCube RadianceEnv			: register(t3);
TextureCube IrradianceEnv		: register(t4);

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
	Out.Albedo = MakeLinearColorFromGammaSpace(DiffuseTexture.Sample(Sampler, Input.UV));
	Out.WorldPos = float4(Input.PosW, Input.PosH.z);

	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);
	float3 normal = normalize((NormalTexture.Sample(Sampler, Input.UV) * 2.0f - 1.0f).xyz);
	float3 worldNormal = mul(normal, TBN);

	Out.NormalW = float4(worldNormal, 1);
	Out.NormalV = float4(mul(worldNormal, (float3x3)viewMatrix), 1);

	return Out;
}

#else

float3 CalculateLightBRDF(float3 lightDir, float3 lightColor, float3 viewDir, float3 normal, float NdotV, float roughness, float alpha, float3 c_diff, float3 c_spec)
{
	// Half vector of light direction and view direction
	float3 H = normalize(lightDir + viewDir);

	float NdotL = saturate(dot(normal, lightDir));
	float LdotH = saturate(dot(lightDir, H));
	float NdotH = saturate(dot(normal, H));

	float diffuse_factor = Diffuse_Burley(NdotL, NdotV, LdotH, roughness);
	float3 specular = Specular_BRDF(alpha, c_spec, NdotV, NdotL, LdotH, NdotH);

	return NdotL * lightColor * ((c_diff * diffuse_factor) + specular);
}

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float4 Final = 0;

	// Sample roughtness, metallic and AO from rgb channels
	float3 RMA = RoughnessMetallicAO.Sample(Sampler, Input.UV).rgb;
	float roughness = RMA.r;
	float metallic = RMA.g;
	float ambientOcclusion = RMA.b;

	//float3 albedo = MakeLinearColorFromGammaSpace(DiffuseTexture.Sample(Sampler, Input.UV)).rgb;
	float3 albedo = DiffuseTexture.Sample(Sampler, Input.UV).rgb;

	static const float kSpecularCoefficient = 0.04;

	float alpha = roughness * roughness;

	float3 c_diff = lerp(albedo, float3(0, 0, 0), metallic);
	float3 c_spec = lerp(kSpecularCoefficient, albedo, metallic);

	float3x3 TBN = CalculateTBNSpace(Input.NormalW, Input.TangentW);

	float3 normal = (NormalTexture.Sample(Sampler, Input.UV) * 2.0f - 1.0f).xyz;
	normal = mul(normal, TBN);

	float3 viewDir = normalize(CameraPos.xyz - Input.PosW);
	float NdotV = saturate(dot(normal, viewDir));

	for (int id = 0; id < DirectionalLightCount; id++)
	{
		float lit = 1.0f;
		float3 lightDir = normalize(DirectionalLight[id].Direction.xyz);

		if (id == 0)
		{
			lit = SampleCascadedShadowMap(Input.ShadowPosH, dot(DirectionalLight[id].Direction.xyz, normal), Input.PosH.z);
		}

		float3 lightColor = DirectionalLight[id].Color.rgb;

		Final.rgb += lit * CalculateLightBRDF(lightDir, lightColor, viewDir, normal, NdotV, roughness, alpha, c_diff, c_spec);
	}

	for (int ip = 0; ip < PointLightCount; ip++)
	{
		float3 lightVec = PointLight[ip].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float attenuation = 1.0f - saturate(length(lightVec) / PointLight[ip].PosAndRadius.w);

		float3 lightColor = PointLight[ip].Color.rgb;

		Final.rgb += attenuation * CalculateLightBRDF(lightDir, lightColor, viewDir, normal, NdotV, roughness, alpha, c_diff, c_spec);
	}

	for (int is = 0; is < SpotlightCount; is++)
	{
		float3 lightVec = Spotlight[is].PosAndRadius.xyz - Input.PosW;
		float3 lightDir = normalize(lightVec);

		float surfaceRatio = saturate(dot(-lightDir, Spotlight[is].Direction.xyz));
		float radiusAtt = 1.0f - saturate(length(lightVec) / Spotlight[is].PosAndRadius.w);
		float coneAtt = 1.0f - saturate((Spotlight[is].ConeRatio.x - surfaceRatio) / (Spotlight[is].ConeRatio.x - Spotlight[is].ConeRatio.y));
		float attenuation = radiusAtt * coneAtt;

		float3 lightColor = Spotlight[is].Color.rgb;

		Final.rgb += attenuation * CalculateLightBRDF(lightDir, lightColor, viewDir, normal, NdotV, roughness, alpha, c_diff, c_spec);
	}

	float3 diffuse_env = Diffuse_IBL(IrradianceEnv, normal);
	Final.rgb += c_diff * diffuse_env;

	float3 specular_env = Specular_IBL(RadianceEnv, normal, viewDir, roughness);
	Final.rgb += c_spec * specular_env;

	Final.a = 1.0f;
	Final.a *= GlobalOpacity;

	return Final;
}

#endif	// #if USE_DEFERRED_SHADING == 1
