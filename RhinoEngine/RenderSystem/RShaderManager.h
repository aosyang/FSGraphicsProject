//=============================================================================
// RShaderManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

enum EShaderFeatureMask
{
	SFM_Skinned		= 1 << 0,
	SFM_Instanced	= 1 << 1,
	SFM_Deferred	= 1 << 2,
};

struct RShader
{
	ID3D11VertexShader*		VertexShader;
	ID3D11PixelShader*		PixelShader;
	ID3D11GeometryShader*	GeometryShader;

	ID3D11VertexShader*		VertexShader_Skinned;
	ID3D11VertexShader*		VertexShader_Instanced;

	ID3D11PixelShader*		PixelShader_Deferred;

	bool operator==(const RShader& rhs) const;

	void Bind(int featureMasks = 0);
	const string& GetName() const;
};

class RShaderManager : public RSingleton<RShaderManager>
{
	friend class RSingleton<RShaderManager>;
	friend struct RShader;
public:

	/// Recursively load all shaders in directory
	void LoadShaders(const char* path);

	/// Unload all loaded shaders
	void UnloadAllShaders();

	bool AddShader(const char* shaderName,
				   const void* pixelShaderBytecode,
				   SIZE_T pixelBytecodeLength,
				   const void* vertexShaderBytecode,
				   SIZE_T vertexBytecodeLength,
				   const void* geometryShaderBytecode = nullptr,
				   SIZE_T geometryBytecodeLength = 0);

	RShader* GetShaderResource(const char* shaderName);

	/// Get a list of all shader names
	vector<string> EnumerateAllShaderNames() const;

	/// Create a pixel shader from bytecode
	ID3D11PixelShader* CreatePixelShaderFromBytecode(const void* pBytecode, SIZE_T BytecodeSize);

private:
	RShaderManager();
	~RShaderManager();

	const string& GetShaderName(const RShader* shader) const;

private:
	map<string, RShader>	m_Shaders;

	static const string EmptyShaderName;
};


FORCEINLINE const string& RShader::GetName() const
{
	return RShaderManager::Instance().GetShaderName(this);
}


