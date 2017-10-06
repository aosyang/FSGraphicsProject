//=============================================================================
// RShaderManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RShaderManager.h"

bool RShader::operator==(const RShader& rhs) const
{
	return
		VertexShader == rhs.VertexShader &&
		VertexShader_Skinned == rhs.VertexShader_Skinned &&
		VertexShader_Instanced == rhs.VertexShader_Instanced &&
		PixelShader == rhs.PixelShader &&
		PixelShader_Deferred == rhs.PixelShader_Deferred &&
		GeometryShader == rhs.GeometryShader;
}

void RShader::Bind(int featureMasks)
{
	if ((featureMasks & SFM_Skinned) && VertexShader_Skinned)
		GRenderer.SetVertexShader(VertexShader_Skinned);
	else if ((featureMasks & SFM_Instanced) && VertexShader_Instanced)
		GRenderer.SetVertexShader(VertexShader_Instanced);
	else
		GRenderer.SetVertexShader(VertexShader);

	if ((featureMasks & SFM_Deferred) && PixelShader_Deferred)
		GRenderer.SetPixelShader(PixelShader_Deferred);
	else
		GRenderer.SetPixelShader(PixelShader);

	GRenderer.SetGeometryShader(GeometryShader);
}

string RShader::GetName() const
{
	return RShaderManager::Instance().GetShaderName(this);
}

RShaderManager::RShaderManager()
{

}

RShaderManager::~RShaderManager()
{

}

void RShaderManager::LoadShaders(const char* path)
{
	// Set working directory to shader folder for compiling
	char pWorkingPath[1024];
	GetCurrentDirectoryA(1024, pWorkingPath);
	SetCurrentDirectoryA(path);

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	// Load shader
	string resFindingPath = "*.hlsl";
	hFind = FindFirstFileA(resFindingPath.data(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				string filename(FindFileData.cFileName);
				string fileFullPath = filename;
				ID3DBlob* pShaderCode = nullptr;
				ID3DBlob* pErrorMsg = nullptr;

				// Get actual shader name
				string shaderName = filename.substr(0, filename.length() - 8);

				if (m_Shaders.find(shaderName) == m_Shaders.end())
				{
					m_Shaders[shaderName] = RShader();
				}

				// Read shader text from .hlsl
				ifstream fin;
				fin.open(fileFullPath, ios_base::binary);

				if (!fin.is_open())
					continue;

				fin.seekg(0, ios_base::end);
				int fileSize = (int)fin.tellg();
				char* pBuffer = new char[fileSize];

				fin.seekg(0);
				fin.read(pBuffer, fileSize);

				int shaderCompileFlag = 0;
#if defined(DEBUG) || defined(_DEBUG)
				shaderCompileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
				HRESULT hr = 0;

#define HANDLE_SHADER_COMPILE_ERROR() \
	if (FAILED(hr)) \
	{ \
		OutputDebugStringA((char*)pErrorMsg->GetBufferPointer()); \
		OutputDebugStringA("\n"); \
	}

				// Detect shader type by file name suffix
				if (filename.find("_VS.hlsl") != string::npos)
				{
					if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
					{
						GRenderer.D3DDevice()->CreateVertexShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].VertexShader);
#ifdef _DEBUG
						m_Shaders[shaderName].VertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)filename.size(), filename.c_str());
#endif
					}
					HANDLE_SHADER_COMPILE_ERROR();

					if (strstr(pBuffer, "USE_SKINNING"))
					{
						filename = string("Skinned") + filename;

						D3D_SHADER_MACRO skinnedShaderMacro[] = { "USE_SKINNING", "1", NULL, NULL };

						if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), skinnedShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
						{
							GRenderer.D3DDevice()->CreateVertexShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].VertexShader_Skinned);
#ifdef _DEBUG
							m_Shaders[shaderName].VertexShader_Skinned->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)filename.size(), filename.c_str());
#endif
						}
						HANDLE_SHADER_COMPILE_ERROR();
					}
					
					if (strstr(pBuffer, "USE_INSTANCING"))
					{
						filename = string("Instanced") + filename;

						D3D_SHADER_MACRO instancedShaderMacro[] = { "USE_INSTANCING", "1", NULL, NULL };

						if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), instancedShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
						{
							GRenderer.D3DDevice()->CreateVertexShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].VertexShader_Instanced);
#ifdef _DEBUG
							m_Shaders[shaderName].VertexShader_Instanced->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)filename.size(), filename.c_str());
#endif
						}
						HANDLE_SHADER_COMPILE_ERROR();
					}
				}
				else if (filename.find("_PS.hlsl") != string::npos)
				{
					if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
					{
						GRenderer.D3DDevice()->CreatePixelShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].PixelShader);
#ifdef _DEBUG
						m_Shaders[shaderName].PixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)filename.size(), filename.c_str());
#endif
					}
					HANDLE_SHADER_COMPILE_ERROR();

					if (strstr(pBuffer, "USE_DEFERRED_SHADING"))
					{
						filename = string("Deferred") + filename;

						D3D_SHADER_MACRO deferredShaderMacro[] = { "USE_DEFERRED_SHADING", "1", NULL, NULL };

						if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), deferredShaderMacro, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
						{
							GRenderer.D3DDevice()->CreatePixelShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].PixelShader_Deferred);
#ifdef _DEBUG
							m_Shaders[shaderName].PixelShader_Deferred->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)filename.size(), filename.c_str());
#endif
						}
						HANDLE_SHADER_COMPILE_ERROR();
					}
				}
				else if (filename.find("_GS.hlsl") != string::npos)
				{
					if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "gs_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
					{
						GRenderer.D3DDevice()->CreateGeometryShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].GeometryShader);
#ifdef _DEBUG
						m_Shaders[shaderName].GeometryShader->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)filename.size(), filename.c_str());
#endif
					}
					HANDLE_SHADER_COMPILE_ERROR();
				}

				delete[] pBuffer;
				SAFE_RELEASE(pShaderCode);
				SAFE_RELEASE(pErrorMsg);
			}

		} while (FindNextFileA(hFind, &FindFileData) != 0);
	}

	// Restore working directory
	SetCurrentDirectoryA(pWorkingPath);
}

void RShaderManager::UnloadAllShaders()
{
	for (map<string, RShader>::iterator iter = m_Shaders.begin();
		 iter != m_Shaders.end(); iter++)
	{
		SAFE_RELEASE(iter->second.VertexShader);
		SAFE_RELEASE(iter->second.VertexShader_Skinned);
		SAFE_RELEASE(iter->second.VertexShader_Instanced);
		SAFE_RELEASE(iter->second.PixelShader);
		SAFE_RELEASE(iter->second.PixelShader_Deferred);
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

	RShader shader = RShader();

	if (pixelShaderBytecode && pixelBytecodeLength)
	{
		GRenderer.D3DDevice()->CreatePixelShader(pixelShaderBytecode, pixelBytecodeLength, NULL, &shader.PixelShader);
	}

	if (vertexShaderBytecode && vertexBytecodeLength)
	{
		GRenderer.D3DDevice()->CreateVertexShader(vertexShaderBytecode, vertexBytecodeLength, NULL, &shader.VertexShader);
	}

	if (geometryShaderBytecode && geometryBytecodeLength)
	{
		GRenderer.D3DDevice()->CreateGeometryShader(geometryShaderBytecode, geometryBytecodeLength, NULL, &shader.GeometryShader);
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

string RShaderManager::GetShaderName(const RShader* shader) const
{
	map<string, RShader>::const_iterator iter = m_Shaders.begin();
	for (; iter != m_Shaders.end(); iter++)
	{
		if (iter->second == *shader)
		{
			return iter->first;
		}
	}

	return "";
}
