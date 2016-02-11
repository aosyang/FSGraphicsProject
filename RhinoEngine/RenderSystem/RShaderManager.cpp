//=============================================================================
// RShaderManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RShaderManager.h"

void RShader::Bind()
{
	RRenderer.D3DImmediateContext()->PSSetShader(PixelShader, NULL, 0);
	RRenderer.D3DImmediateContext()->VSSetShader(VertexShader, NULL, 0);
	RRenderer.D3DImmediateContext()->GSSetShader(GeometryShader, NULL, 0);
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
		SAFE_RELEASE(iter->second.PixelShader);
		SAFE_RELEASE(iter->second.VertexShader);
		SAFE_RELEASE(iter->second.GeometryShader);
	}

	m_Shaders.clear();
}

bool RShaderManager::AddShader(const char* shaderName,
							   const void* pixelShaderBytecode,
							   SIZE_T pixelBytecodeLength,
							   const void* vertexShaderBytecode,
							   SIZE_T vertexBytecodeLength,
							   const void* geometryShaderBytecode,
							   SIZE_T geometryBytecodeLength)
{
	if (m_Shaders.find(shaderName) != m_Shaders.end())
		return false;

	RShader shader;

	shader.PixelShader = nullptr;
	shader.VertexShader = nullptr;
	shader.GeometryShader = nullptr;

	if (pixelShaderBytecode && pixelBytecodeLength)
	{
		RRenderer.D3DDevice()->CreatePixelShader(pixelShaderBytecode, pixelBytecodeLength, NULL, &shader.PixelShader);
		shader.PS_Bytecode = (BYTE*)pixelShaderBytecode;
		shader.PS_BytecodeSize = pixelBytecodeLength;
	}

	if (vertexShaderBytecode && vertexBytecodeLength)
	{
		RRenderer.D3DDevice()->CreateVertexShader(vertexShaderBytecode, vertexBytecodeLength, NULL, &shader.VertexShader);
		shader.VS_Bytecode = (BYTE*)vertexShaderBytecode;
		shader.VS_BytecodeSize = vertexBytecodeLength;
	}

	if (geometryShaderBytecode && geometryBytecodeLength)
	{
		RRenderer.D3DDevice()->CreateGeometryShader(geometryShaderBytecode, geometryBytecodeLength, NULL, &shader.GeometryShader);
		shader.GS_Bytecode = (BYTE*)geometryShaderBytecode;
		shader.GS_BytecodeSize = geometryBytecodeLength;
	}

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
