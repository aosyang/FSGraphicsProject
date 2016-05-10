//=============================================================================
// ConstBufferPS.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _CONSTBUFFERPS_H
#define _CONSTBUFFERPS_H

#include "SharedDefines.h"

#define MAX_DIRECTIONAL_LIGHT 3
#define MAX_POINT_LIGHT 8
#define MAX_SPOTLIGHT 3

struct DIRECTIONAL_LIGHT
{
	float4	Direction;
	float4	Color;				// w as color intensity
};

struct POINT_LIGHT
{
	float4	PosAndRadius;		// w as light radius
	float4	Color;				// w as color intensity
};

struct SPOTLIGHT
{
	float4	PosAndRadius;				// w as light radius
	float4	Direction;
	float4	ConeRatio;					// x, y as inner/outer cone ratio
	float4	Color;						// w as color intensity
};

CONSTANT_BUFFER_BEGIN(SHADER_LIGHT_BUFFER, b0)
DIRECTIONAL_LIGHT	DirectionalLight[MAX_DIRECTIONAL_LIGHT];
POINT_LIGHT			PointLight[MAX_POINT_LIGHT];
SPOTLIGHT			Spotlight[MAX_SPOTLIGHT];

float4				CameraPos;
float4				HighHemisphereAmbientColor;
float4				LowHemisphereAmbientColor;

// Note: An int type is 16 byte aligned in HLSL and 4 byte aligned in C++
int					DirectionalLightCount;
int					PointLightCount;
int					SpotlightCount;
int					CascadedShadowCount;
CONSTANT_BUFFER_END

CONSTANT_BUFFER_BEGIN(SHADER_MATERIAL_BUFFER, b1)
float4				SpecularColorAndPower;
float				GlobalOpacity;
CONSTANT_BUFFER_END

CONSTANT_BUFFER_BEGIN(SHADER_DEFERRED_POINT_LIGHT_BUFFER, b2)
POINT_LIGHT			DeferredPointLight;
CONSTANT_BUFFER_END

#ifndef __cplusplus
float4 MakeLinearColorFromGammaSpace(float4 color)
{
	if (UseGammaCorrection)
		return float4(pow(color.rgb, 2.2f), color.a);
	else
		return color;
}
#endif

#endif