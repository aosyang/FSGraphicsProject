//=============================================================================
// ConstBufferVS.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _CONSTBUFFERVS_H
#define _CONSTBUFFERVS_H

#include "SharedDefines.h"
#include "ConstBufferCommon.h"

CONSTANT_BUFFER_BEGIN(SHADER_OBJECT_BUFFER, b2)
float4x4			worldMatrix;
CONSTANT_BUFFER_END

#define MAX_INSTANCE_COUNT 128

CONSTANT_BUFFER_BEGIN(SHADER_INSTANCE_BUFFER, b3)
float4x4	instancedWorldMatrix[MAX_INSTANCE_COUNT];
CONSTANT_BUFFER_END

#define MAX_BONE_COUNT 192

CONSTANT_BUFFER_BEGIN(SHADER_SKINNED_BUFFER, b4)
float4x4	boneMatrix[MAX_BONE_COUNT];
CONSTANT_BUFFER_END


#endif