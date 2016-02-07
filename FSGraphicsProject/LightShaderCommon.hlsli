//=============================================================================
// LightShaderCommon.hlsli by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _LIGHTSHADERCOMMON_HLSLI
#define _LIGHTSHADERCOMMON_HLSLI

float3 CalculateDiffuseLight(float3 normal,
							 float3 lightDir,
							 float4 lightColor)
{
	float DiffuseIntensity = dot(normal, lightDir);
	return saturate(DiffuseIntensity) * lightColor.rgb * lightColor.a;
}

float3 CalculateSpecularLight(float3 normal,
							  float3 lightDir,
							  float3 viewDir,
							  float4 lightColor,
							  float4 specularColor)
{
	//float3 halfVec = normalize(lightDir + viewDir);
	//float SpecularIntensity = max(pow(saturate(dot(normal, normalize(halfVec))), specularColor.w), 0.0f);
	float3 lightReflect = normalize(reflect(-lightDir, normal));
	float SpecularIntensity = max(pow(saturate(dot(viewDir, lightReflect)), specularColor.w), 0.0f);
	return lightColor.rgb * specularColor.rgb * SpecularIntensity;
}

#endif
