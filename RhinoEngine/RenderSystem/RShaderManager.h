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

enum class EShaderType : UINT8
{
	Unknown,
	VertexShader,
	PixelShader,
	GeometryShader,
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
	const std::string& GetName() const;
};

class RShaderManager : public RSingleton<RShaderManager>
{
	friend class RSingleton<RShaderManager>;
	friend struct RShader;
public:
	/// Get the root path of the shader folder
	static std::string GetShaderRootPath();

	/// Recursively load all shaders in directory
	void LoadShaders(const std::string& Path);

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
	std::vector<std::string> EnumerateAllShaderNames() const;

	/// Create a pixel shader from bytecode
	ID3D11PixelShader* CreatePixelShaderFromBytecode(const void* pBytecode, SIZE_T BytecodeSize);

private:
	RShaderManager();
	~RShaderManager();

	const std::string& GetShaderName(const RShader* shader) const;

	void CompileShader(const std::string& SourceName, const char* pBuffer, int BufferSize, RShader* Shader);

	void CreateVertexShader(const std::string& SourceName, const void* ShaderBytecode, SIZE_T BytecodeLength, ID3D11VertexShader** VertexShader);
	void CreatePixelShader(const std::string& SourceName, const void* ShaderBytecode, SIZE_T BytecodeLength, ID3D11PixelShader** PixelShader);
	void CreateGeometryShader(const std::string& SourceName, const void* ShaderBytecode, SIZE_T BytecodeLength, ID3D11GeometryShader** GeometryShader);

	/// Load a shader from its cache file
	bool TryLoadShaderFromCache(const std::string& SourceName, const std::string& DiskFileName, std::vector<char>& OutBytecode);

	/// Save cache file for a shader to disk
	void SaveShaderCache(const std::string& SourceName, const void* ShaderBytecode, SIZE_T BytecodeLength);

	/// Generate cache file name for a shader file
	std::string MakeCacheFileName(const std::string& SourceName) const;

	/// Get the path of shader cache folder
	std::string GetShaderCachePath() const;

	/// Guess type of a shader by its file name
	EShaderType DetectShaderType(const std::string& FileName) const;
	UINT GetShaderCompileFlag() const;

private:
	std::map<std::string, RShader>	m_Shaders;

	static const std::string EmptyShaderName;
};


FORCEINLINE const std::string& RShader::GetName() const
{
	return RShaderManager::Instance().GetShaderName(this);
}


