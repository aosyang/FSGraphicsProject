//=============================================================================
// RShaderManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RShaderManager.h"

bool RShader::operator==(const RShader& rhs) const
{
	return PixelShader == rhs.PixelShader &&
		VertexShader == rhs.VertexShader &&
		GeometryShader == rhs.GeometryShader;
}

void RShader::Bind()
{
	RRenderer.D3DImmediateContext()->PSSetShader(PixelShader, NULL, 0);
	RRenderer.D3DImmediateContext()->VSSetShader(VertexShader, NULL, 0);
	RRenderer.D3DImmediateContext()->GSSetShader(GeometryShader, NULL, 0);
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

				// Detect shader type by file name suffix
				if (filename.find("_VS.hlsl") != string::npos)
				{
					if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
					{
						RRenderer.D3DDevice()->CreateVertexShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].VertexShader);
					}
				}
				else if (filename.find("_PS.hlsl") != string::npos)
				{
					if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
					{
						RRenderer.D3DDevice()->CreatePixelShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].PixelShader);
					}
				}
				else if (filename.find("_GS.hlsl") != string::npos)
				{
					if (SUCCEEDED(hr = D3DCompile(pBuffer, fileSize, filename.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "gs_4_0", shaderCompileFlag, 0, &pShaderCode, &pErrorMsg)))
					{
						RRenderer.D3DDevice()->CreateGeometryShader(pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), NULL, &m_Shaders[shaderName].GeometryShader);
					}
				}

				if (FAILED(hr))
				{
					OutputDebugStringA((char*)pErrorMsg->GetBufferPointer());
					OutputDebugStringA("\n");
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
	}

	if (vertexShaderBytecode && vertexBytecodeLength)
	{
		RRenderer.D3DDevice()->CreateVertexShader(vertexShaderBytecode, vertexBytecodeLength, NULL, &shader.VertexShader);
	}

	if (geometryShaderBytecode && geometryBytecodeLength)
	{
		RRenderer.D3DDevice()->CreateGeometryShader(geometryShaderBytecode, geometryBytecodeLength, NULL, &shader.GeometryShader);
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
