//=============================================================================
// RShaderManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#include "RShaderManager.h"

void RShader::Bind()
{
	RRenderer.D3DImmediateContext()->PSSetShader(PixelShader, NULL, 0);
	RRenderer.D3DImmediateContext()->VSSetShader(VertexShader, NULL, 0);
}

RShaderManager::RShaderManager()
{

}

RShaderManager::~RShaderManager()
{

}

void RShaderManager::UnloadAllShaders()
{
	for (map<string, RShader>::iterator iter = m_Shaders.begin();
		 iter != m_Shaders.end(); iter++)
	{
		iter->second.PixelShader->Release();
		iter->second.VertexShader->Release();
	}

	m_Shaders.clear();
}

bool RShaderManager::AddShader(const char* shaderName,
							   const void* pixelShaderBytecode,
							   SIZE_T pixelBytecodeLength,
							   const void* vertexShaderBytecode,
							   SIZE_T vertexBytecodeLength)
{
	if (m_Shaders.find(shaderName) != m_Shaders.end())
		return false;

	RShader shader;
	RRenderer.D3DDevice()->CreatePixelShader(pixelShaderBytecode, pixelBytecodeLength, NULL, &shader.PixelShader);
	RRenderer.D3DDevice()->CreateVertexShader(vertexShaderBytecode, vertexBytecodeLength, NULL, &shader.VertexShader);

	shader.PS_Bytecode = (BYTE*)pixelShaderBytecode;
	shader.PS_BytecodeSize = pixelBytecodeLength;
	shader.VS_Bytecode = (BYTE*)vertexShaderBytecode;
	shader.VS_BytecodeSize = vertexBytecodeLength;

	m_Shaders[shaderName] = shader;

	return true;
}

RShader* RShaderManager::GetShaderResource(const char* shaderName)
{
	map<string, RShader>::iterator iter = m_Shaders.find(shaderName);
	if (iter != m_Shaders.end())
		return &iter->second;
	return nullptr;
}