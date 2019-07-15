//=============================================================================
// RShaderManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RShaderManager.h"

const string RShaderManager::EmptyShaderName = "";

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

RShaderManager::RShaderManager()
{

}

RShaderManager::~RShaderManager()
{

}

string RShaderManager::GetShaderRootPath()
{
	string Path("../Shaders");
	if (PathFileExistsA(Path.c_str()) == FALSE)
	{
		Path = string("../") + Path;
		if (PathFileExistsA(Path.c_str()) == FALSE)
		{
			return RFileUtil::InvalidPath;
		}
	}

	return RFileUtil::GetFullPath(Path);
}

void RShaderManager::LoadShaders(const string& Path)
{
	if (PathFileExistsA(Path.c_str()) == FALSE)
	{
		string FullPath = RFileUtil::GetFullPath(Path);
		RLogError("ShaderManager: Path \'%s\' does not exist while loading shaders.\n", FullPath.c_str());
		return;
	}

	// Set working directory to shader folder for compiling
	RFileUtil::PushWorkingPath(Path);

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
				fin.close();

#if defined(DEBUG) || defined(_DEBUG)
				int shaderCompileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
				int shaderCompileFlag = 0;
#endif
				HRESULT hr = 0;

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
					else
					{
						RLog("%s\n", (char*)pErrorMsg->GetBufferPointer());
					}

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
						else
						{
							RLog("%s\n", (char*)pErrorMsg->GetBufferPointer());
						}
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
						else
						{
							RLog("%s\n", (char*)pErrorMsg->GetBufferPointer());
						}
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
					else
					{
						RLog("%s\n", (char*)pErrorMsg->GetBufferPointer());
					}

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
						else
						{
							RLog("%s\n", (char*)pErrorMsg->GetBufferPointer());
						}
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
					else
					{
						RLog("%s\n", (char*)pErrorMsg->GetBufferPointer());
					}
				}

				delete[] pBuffer;
				SAFE_RELEASE(pShaderCode);
				SAFE_RELEASE(pErrorMsg);
			}

		} while (FindNextFileA(hFind, &FindFileData) != 0);
	}

	// Restore working directory
	RFileUtil::PopWorkingPath();
}

void RShaderManager::UnloadAllShaders()
{
	for (auto Iter : m_Shaders)
	{
		SAFE_RELEASE(Iter.second.VertexShader);
		SAFE_RELEASE(Iter.second.VertexShader_Skinned);
		SAFE_RELEASE(Iter.second.VertexShader_Instanced);
		SAFE_RELEASE(Iter.second.PixelShader);
		SAFE_RELEASE(Iter.second.PixelShader_Deferred);
		SAFE_RELEASE(Iter.second.GeometryShader);
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
	auto iter = m_Shaders.find(shaderName);
	if (iter != m_Shaders.end())
		return &iter->second;
	return nullptr;
}

vector<string> RShaderManager::EnumerateAllShaderNames() const
{
	vector<string> NameList;

	for (auto& ShaderData : m_Shaders)
	{
		NameList.push_back(ShaderData.first);
	}

	return NameList;
}

ID3D11PixelShader* RShaderManager::CreatePixelShaderFromBytecode(const void* pBytecode, SIZE_T BytecodeSize)
{
	ID3D11PixelShader* OutputShader = nullptr;
	if (FAILED(GRenderer.D3DDevice()->CreatePixelShader(pBytecode, BytecodeSize, 0, &OutputShader)))
	{
		RLogWarning("Failed to create pixel shader from bytecode!\n");
		return nullptr;
	}

	return OutputShader;
}

const string& RShaderManager::GetShaderName(const RShader* shader) const
{
	auto iter = m_Shaders.begin();
	for (; iter != m_Shaders.end(); iter++)
	{
		if (iter->second == *shader)
		{
			return iter->first;
		}
	}

	return EmptyShaderName;
}
