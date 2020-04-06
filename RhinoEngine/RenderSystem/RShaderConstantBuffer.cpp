//=============================================================================
// RShaderConstantBuffer.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "RShaderConstantBuffer.h"

RShaderConstantBuffer<SHADER_SCENE_BUFFER,		CBST_VS|CBST_GS|CBST_PS, 0>		RConstantBuffers::cbScene;
RShaderConstantBuffer<SHADER_GLOBAL_BUFFER,		CBST_VS|CBST_PS, 1>				RConstantBuffers::cbGlobal;
RShaderConstantBuffer<SHADER_OBJECT_BUFFER,		CBST_VS, 2>						RConstantBuffers::cbPerObject;
RShaderConstantBuffer<SHADER_SKINNED_BUFFER,	CBST_VS, 4>						RConstantBuffers::cbBoneMatrices;
RShaderConstantBuffer<SHADER_LIGHT_BUFFER,		CBST_PS, 2>						RConstantBuffers::cbLight;
RShaderConstantBuffer<SHADER_MATERIAL_BUFFER,	CBST_PS, 3>						RConstantBuffers::cbMaterial;
