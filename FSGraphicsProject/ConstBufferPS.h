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

struct DIRECTIONAL_LIGHT
{
	float4	Direction;
	float4	Color;			// w as color ratio
};

struct POINT_LIGHT
{
	float4	PosAndRadius;	// w as light radius
	float4	Color;			// w as color ratio
};

CONSTANT_BUFFER_BEGIN(SHADER_LIGHT_BUFFER, b0)
DIRECTIONAL_LIGHT	DirectionalLight[MAX_DIRECTIONAL_LIGHT];
POINT_LIGHT			PointLight[MAX_POINT_LIGHT];

// Note: An int type is 16 byte aligned in HLSL and 4 byte aligned in C++
int					DirectionalLightCount;
int					PointLightCount;
CONSTANT_BUFFER_END

#endif