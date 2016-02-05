// This file defines how code can be used with C++ and HLSL
#ifndef SHARED_DEFINES_H
#define SHARED_DEFINES_H

#ifdef __cplusplus
	#pragma once
	#include <DirectXMath.h>
	typedef unsigned int		uint;
	typedef DirectX::XMFLOAT2	float2;
	typedef DirectX::XMFLOAT3	float3;
	typedef DirectX::XMFLOAT4	float4;
	typedef DirectX::XMFLOAT4X4	float4x4;
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

// Shader constant buffers
CONSTANT_BUFFER_BEGIN(SHADER_OBJECT_BUFFER, b0)
float4x4	worldMatrix;
CONSTANT_BUFFER_END

CONSTANT_BUFFER_BEGIN(SHADER_SCENE_BUFFER, b1)
float4x4	viewMatrix;
float4x4	projMatrix;
float4x4	viewProjMatrix;
float4		cameraPos;
CONSTANT_BUFFER_END

#endif //SHARED_DEFINES_H