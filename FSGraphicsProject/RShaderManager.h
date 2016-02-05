//=============================================================================
// RShaderManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSHADERMANAGER_H
#define _RSHADERMANAGER_H

#include "Rhino.h"
#include <map>
#include <string>

using namespace std;

struct RShader
{
	ID3D11PixelShader*	PixelShader;
	ID3D11VertexShader*	VertexShader;

	BYTE*				PS_Bytecode;
	UINT32				PS_BytecodeSize;
	BYTE*				VS_Bytecode;
	UINT32				VS_BytecodeSize;

	void Bind();
};

class RShaderManager : public RSingleton<RShaderManager>
{
	friend class RSingleton<RShaderManager>;
public:

	void Initialize();
	void UnloadAllShaders();

	RShader* GetShaderResource(const char* shaderName);

private:
	RShaderManager();
	~RShaderManager();

	bool AddShader(const char* shaderName,
				   const void* pixelShaderBytecode,
				   SIZE_T pixelBytecodeLength,
				   const void* vertexShaderBytecode,
				   SIZE_T vertexBytecodeLength);

	map<string, RShader>	m_Shaders;
};

#endif
