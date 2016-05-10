// This file defines how code can be used with C++ and HLSL
#ifndef SHARED_DEFINES_H
#define SHARED_DEFINES_H

#ifdef __cplusplus
	#pragma once
	#include "Rhino.h"
	typedef unsigned int		uint;
	typedef RVec2				float2;
	typedef RVec3				float3;
	typedef RVec4				float4;
	typedef RMatrix4			float4x4;
	// lets us ensure constant buffers and their variables are 16byte aligned to HLSL 4-float registers
	#define _regAlign __declspec(align(16))
	// allows us to attach semantics to HLSL variables without bugging C++
	#define SEMANTIC(s_name) /* : s_name */

	#define CONSTANT_BUFFER_BEGIN(cb_name, reg) struct _regAlign cb_name {
	#define CONSTANT_BUFFER_END };
#else
	#pragma pack_matrix(row_major)

	// lets us ensure constant buffers and variables are 16byte aligned (HLSL will do this for us anyway)
	#define _regAlign /**/
	// allows us to attach semantics to HLSL variables without bugging C++
	#define SEMANTIC(s_name) : s_name
	// In HLSL constant buffers will be identified by their name and sequential ordering
	#define CONSTANT_BUFFER_BEGIN(cb_name, reg) cbuffer cb_name : register(reg){
	//#define CONSTANT_BUFFER_BEGIN(cb_name) cbuffer cb_name {
	#define CONSTANT_BUFFER_END }
#endif

CONSTANT_BUFFER_BEGIN(SHADER_GLOBAL_BUFFER, b4)
float2				ScreenSize;
bool				UseGammaCorrection;
CONSTANT_BUFFER_END

#endif //SHARED_DEFINES_H