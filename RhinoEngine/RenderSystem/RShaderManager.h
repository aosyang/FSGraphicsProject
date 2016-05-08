//=============================================================================
// RShaderManager.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSHADERMANAGER_H
#define _RSHADERMANAGER_H

enum EShaderFeatureMask
{
	SFM_Skinned = 1 << 0,
};

struct RShader
{
	ID3D11VertexShader*		VertexShader[2];
	ID3D11PixelShader*		PixelShader;
	ID3D11GeometryShader*	GeometryShader;

	bool operator==(const RShader& rhs) const;

	void Bind(int featureMasks = 0);
	string GetName() const;
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
	string GetShaderName(const RShader* shader) const;

private:
	RShaderManager();
	~RShaderManager();

	map<string, RShader>	m_Shaders;
};

#endif
