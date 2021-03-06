//=============================================================================
// PixelShaderCommon.hlsli by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _PIXELSHADERCOMMON_H
#define _PIXELSHADERCOMMON_H

SamplerState Sampler : register(s0);

float4 MakeLinearColorFromGammaSpace(float4 color)
{
	if (UseGammaCorrection)
		return float4(pow(max(color.rgb, 0), 2.2f), color.a);
	else
		return color;
}

float LinearizeDepth(float z)
{
	float c1 = ClipPlaneNearFar.y / ClipPlaneNearFar.x;
	float c0 = 1.0 - c1;
	return 1.0 / (c0 * z + c1);
}

float LinearizeDepth(float near, float far, float z)
{
	float c1 = far / near;
	float c0 = 1.0 - c1;
	return 1.0 / (c0 * z + c1);
}

// Returns uv with global tiling factor applied
float2 GetTiledUV(float2 InUV)
{
	return InUV * UVTiling;
}

#endif