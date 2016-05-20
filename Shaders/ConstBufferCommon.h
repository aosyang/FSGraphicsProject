//=============================================================================
// ConstBufferCommon.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _CONSTBUFFERCOMMON_H
#define _CONSTBUFFERCOMMON_H


CONSTANT_BUFFER_BEGIN(SHADER_SCENE_BUFFER, b0)
float4x4	viewMatrix;
float4x4	cameraMatrix;			// Inverse view matrix
float4x4	projMatrix;
float4x4	viewProjMatrix;
float4x4	invProjMatrix;
float4		cameraPos;
float4x4	shadowViewProjMatrix[3];
float4x4	shadowViewProjBiasedMatrix[3];
int			cascadedShadowIndex;
CONSTANT_BUFFER_END

CONSTANT_BUFFER_BEGIN(SHADER_GLOBAL_BUFFER, b1)
float4				ScreenSize;					// z and w for texel size
float4x4			ViewToTextureSpace;			// For screen space reflection
float2				ClipPlaneNearFar;
bool				UseGammaCorrection;
float				TotalTime;
CONSTANT_BUFFER_END


#endif