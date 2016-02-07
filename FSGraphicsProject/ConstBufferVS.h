//=============================================================================
// ConstBufferVS.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _CONSTBUFFERVS_H
#define _CONSTBUFFERVS_H

#include "SharedDefines.h"

CONSTANT_BUFFER_BEGIN(SHADER_OBJECT_BUFFER, b0)
float4x4			worldMatrix;
CONSTANT_BUFFER_END

CONSTANT_BUFFER_BEGIN(SHADER_SCENE_BUFFER, b1)
float4x4	viewMatrix;
float4x4	projMatrix;
float4x4	viewProjMatrix;
float4		cameraPos;
CONSTANT_BUFFER_END

#endif