//=============================================================================
// RShaderManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSHADERMANAGER_H
#define _RSHADERMANAGER_H

struct RShader
{
	ID3D11PixelShader*		PixelShader;
	ID3D11VertexShader*		VertexShader;
	ID3D11GeometryShader*	GeometryShader;

	BYTE*					PS_Bytecode;
	UINT32					PS_BytecodeSize;
	BYTE*					VS_Bytecode;
	UINT32					VS_BytecodeSize;
	BYTE*					GS_Bytecode;
	UINT32					GS_BytecodeSize;

	void Bind();
};

class RShaderManager : public RSingleton<RShaderManager>
{
	friend class RSingleton<RShaderManager>;
public:

	void LoadShaders(const char* path);
	void UnloadAllShaders();

	bool AddShader(const char* shaderName,
				   const void* pixelShaderBytecode,
				   SIZE_T pixelBytecodeLength,
				   const void* vertexShaderBytecode,
				   SIZE_T vertexBytecodeLength,
				   const void* geometryShaderBytecode = nullptr,
				   SIZE_T geometryBytecodeLength = 0);

	RShader* GetShaderResource(const char* shaderName);

private:
	RShaderManager();
	~RShaderManager();

	map<string, RShader>	m_Shaders;
};

#endif
