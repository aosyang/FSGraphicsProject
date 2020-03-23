//=============================================================================
// WorldAligned_PS.hlsl by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "ConstBufferPS.h"
#include "PixelShaderCommon.hlsli"

#define PHONG_NO_SPECULAR 1
#include "LightingModel_Phong.hlsli"

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

// Get a unit vector in the direction of the longest axis
float3 GetMaxAxisDirection(float3 v)
{
	float x = abs(v.x);
	float y = abs(v.y);
	float z = abs(v.z);

	if (x > y)
	{
		if (x > z)
		{
			return float3(1, 0, 0);
		}
		else
		{
			return float3(0, 0, 1);
		}
	}
	else
	{
		if (y > z)
		{
			return float3(0, 1, 0);
		}
		else
		{
			return float3(0, 0, 1);
		}
	}
}

float4 SampleWorldSpaceTexture(Texture2D Texture, float3 WorldNormal, float3 WorldPos)
{
	float3 MaxDir = GetMaxAxisDirection(WorldNormal);
	return Texture.Sample(Sampler, WorldPos.yz / 200) * MaxDir.x +
		   Texture.Sample(Sampler, WorldPos.xz / 200) * MaxDir.y +
		   Texture.Sample(Sampler, WorldPos.xy / 200) * MaxDir.z;
}

#if USE_DEFERRED_SHADING == 1

struct OUTPUT_PIXEL
{
	float4 Albedo		: SV_Target0;
	float4 WorldPos		: SV_Target1;
	float4 NormalW		: SV_Target2;
	float4 NormalV		: SV_Target3;
};

OUTPUT_PIXEL main(OUTPUT_VERTEX Input)
{
	OUTPUT_PIXEL Out = (OUTPUT_PIXEL)0;
	Out.Albedo = SampleWorldSpaceTexture(DiffuseTexture, normalize(Input.NormalW), Input.PosW);
	Out.WorldPos = float4(Input.PosW, Input.PosH.z);
	Out.NormalW = float4(normalize(Input.NormalW), 1);
	Out.NormalV = float4(normalize(Input.NormalV), 1);

	return Out;
}

#else

float4 main(OUTPUT_VERTEX Input) : SV_TARGET
{
	float3 normal = normalize(Input.NormalW);

	Input.ShadowPosH[0].xyz /= Input.ShadowPosH[0].w;
	Input.ShadowPosH[1].xyz /= Input.ShadowPosH[1].w;
	Input.ShadowPosH[2].xyz /= Input.ShadowPosH[2].w;

	PhongLightingData LightColor = CalculatePhongLighting(Input.PosW, normal, Input.ShadowPosH, Input.PosH.z);

	// Hemisphere ambient color
	float3 Ambient = CalculateAmbientLight(normal, HighHemisphereAmbientColor, LowHemisphereAmbientColor);

	// Color from diffuse texture
	float4 DiffuseSample = SampleWorldSpaceTexture(DiffuseTexture, normal, Input.PosW);

	// Compose the final color
	float4 Final;
	Final.rgb = (Ambient + LightColor.Diffuse) * DiffuseSample.rgb;
	Final.a = DiffuseSample.a * GlobalOpacity;
	return Final;
}

#endif	// #if USE_DEFERRED_SHADING == 1
