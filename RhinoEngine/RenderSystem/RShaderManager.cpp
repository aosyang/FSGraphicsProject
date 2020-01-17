//=============================================================================
// RShaderManager.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RShaderManager.h"
#include "D3DCommonPrivate.h"

struct RShaderCompileOption
{
	EShaderType			Type;
	std::string			Prefix;
	const char*			ShaderTarget;
	D3D_SHADER_MACRO	ShaderMacros[2];
	int					MemberOffset;
};

namespace
{
	// Compile options for all shaders and their variations
	RShaderCompileOption ShaderCompileOptions[] =
	{
		{ EShaderType::VertexShader,	"",				"vs_4_0", { nullptr, nullptr },								offsetof(RShader, VertexShader) },
		{ EShaderType::VertexShader,	"Skinned",		"vs_4_0", { "USE_SKINNING",	  "1", nullptr, nullptr },		offsetof(RShader, VertexShader_Skinned) },
		{ EShaderType::VertexShader,	"Instanced",	"vs_4_0", { "USE_INSTANCING", "1", nullptr, nullptr },		offsetof(RShader, VertexShader_Instanced) },
		{ EShaderType::PixelShader,		"",				"ps_4_0", { nullptr, nullptr },								offsetof(RShader, PixelShader) },
		{ EShaderType::PixelShader,		"Deferred",		"ps_4_0", { "USE_DEFERRED_SHADING", "1", nullptr, nullptr}, offsetof(RShader, PixelShader_Deferred) },
		{ EShaderType::GeometryShader,	"",				"gs_4_0", { nullptr, nullptr },								offsetof(RShader, GeometryShader) },
	};
}

const std::string RShaderManager::EmptyShaderName = "";

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

std::string RShaderManager::GetShaderRootPath()
{
	std::string Path("../Shaders");
	if (PathFileExistsA(Path.c_str()) == FALSE)
	{
		Path = std::string("../") + Path;
		if (PathFileExistsA(Path.c_str()) == FALSE)
		{
			return RFileUtil::InvalidPath;
		}
	}

	return RFileUtil::GetFullPath(Path);
}

void RShaderManager::LoadShaders(const std::string& Path)
{
	if (PathFileExistsA(Path.c_str()) == FALSE)
	{
		std::string FullPath = RFileUtil::GetFullPath(Path);
		RLogError("ShaderManager: Path \'%s\' does not exist while loading shaders.\n", FullPath.c_str());
		return;
	}

	// Set working directory to shader folder for compiling
	RFileUtil::PushWorkingPath(Path);

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	// Load shader
	std::string resFindingPath = "*.hlsl";
	hFind = FindFirstFileA(resFindingPath.data(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				std::string filename(FindFileData.cFileName);

				// Get actual shader name
				std::string shaderName = filename.substr(0, filename.length() - 8);

				RShader* Shader = nullptr;
				auto Iter = m_Shaders.find(shaderName);
				if (Iter == m_Shaders.end())
				{
					auto PairIter = m_Shaders.emplace(shaderName, RShader{});
					if (PairIter.second == true)
					{
						Shader = &PairIter.first->second;
					}
				}
				else
				{
					Shader = &Iter->second;
				}

				if (!Shader)
				{
					RLogError("RShaderManager::LoadShaders - Failed to allocate a new shader!");
					continue;
				}

				// Read shader text from .hlsl
				std::ifstream fin;
				std::string fileFullPath = filename;
				fin.open(fileFullPath, std::ios_base::binary);

				if (!fin.is_open())
					continue;

				fin.seekg(0, std::ios_base::end);
				int fileSize = (int)fin.tellg();
				char* pBuffer = new char[fileSize];

				fin.seekg(0);
				fin.read(pBuffer, fileSize);
				fin.close();

				RLog("Compiling shader %s\n", filename.c_str());
				CompileShader(filename, pBuffer, fileSize, Shader);
				delete[] pBuffer;
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

std::vector<std::string> RShaderManager::EnumerateAllShaderNames() const
{
	std::vector<std::string> NameList;

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

const std::string& RShaderManager::GetShaderName(const RShader* shader) const
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

void RShaderManager::CompileShader(const std::string& SourceName, const char* pBuffer, int BufferSize, RShader* Shader)
{
	HRESULT hr;
	ComPtr<ID3DBlob> pShaderCode;
	ComPtr<ID3DBlob> pErrorMsg;

	EShaderType ShaderType = DetectShaderType(SourceName);
	if (ShaderType == EShaderType::Unknown)
	{
		return;
	}

	for (int Index = 0; Index < ARRAYSIZE(ShaderCompileOptions); Index++)
	{
		const RShaderCompileOption& CompileOption = ShaderCompileOptions[Index];

		if (ShaderType != CompileOption.Type)
		{
			continue;
		}

		if (!CompileOption.ShaderMacros[0].Name || strstr(pBuffer, CompileOption.ShaderMacros[0].Name))
		{
			std::string ActualSourceName = CompileOption.Prefix + SourceName;

			if (SUCCEEDED(hr = D3DCompile(pBuffer, BufferSize, ActualSourceName.c_str(), CompileOption.ShaderMacros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", CompileOption.ShaderTarget, GetShaderCompileFlag(), 0, &pShaderCode, &pErrorMsg)))
			{
				switch (ShaderType)
				{
				case EShaderType::VertexShader:
					CreateVertexShader(ActualSourceName, pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), (ID3D11VertexShader**)((char*)Shader + CompileOption.MemberOffset));
					break;

				case EShaderType::PixelShader:
					CreatePixelShader(ActualSourceName, pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), (ID3D11PixelShader**)((char*)Shader + CompileOption.MemberOffset));
					break;

				case EShaderType::GeometryShader:
					CreateGeometryShader(ActualSourceName, pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), (ID3D11GeometryShader**)((char*)Shader + CompileOption.MemberOffset));
					break;
				}
			}
			else
			{
				RLog("%s\n", (char*)pErrorMsg->GetBufferPointer());
			}
		}
	}
}

void RShaderManager::CreateVertexShader(const std::string& SourceName, const void* ShaderBytecode, SIZE_T BytecodeLength, ID3D11VertexShader** VertexShader)
{
	if (SUCCEEDED(GRenderer.D3DDevice()->CreateVertexShader(ShaderBytecode, BytecodeLength, NULL, VertexShader)))
	{
#ifdef _DEBUG
		// Assign source file name to shader for debugging
		(*VertexShader)->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)SourceName.size(), SourceName.c_str());
#endif
	}
}

void RShaderManager::CreatePixelShader(const std::string& SourceName, const void* ShaderBytecode, SIZE_T BytecodeLength, ID3D11PixelShader** PixelShader)
{
	if (SUCCEEDED(GRenderer.D3DDevice()->CreatePixelShader(ShaderBytecode, BytecodeLength, NULL, PixelShader)))
	{
#ifdef _DEBUG
		// Assign source file name to shader for debugging
		(*PixelShader)->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)SourceName.size(), SourceName.c_str());
#endif
	}
}

void RShaderManager::CreateGeometryShader(const std::string& SourceName, const void* ShaderBytecode, SIZE_T BytecodeLength, ID3D11GeometryShader** GeometryShader)
{
	if (SUCCEEDED(GRenderer.D3DDevice()->CreateGeometryShader(ShaderBytecode, BytecodeLength, NULL, GeometryShader)))
	{
#ifdef _DEBUG
		// Assign source file name to shader for debugging
		(*GeometryShader)->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)SourceName.size(), SourceName.c_str());
#endif
	}
}

EShaderType RShaderManager::DetectShaderType(const std::string& FileName) const
{
	if (FileName.find("_VS") != std::string::npos)
	{
		return EShaderType::VertexShader;
	}
	else if (FileName.find("_PS") != std::string::npos)
	{
		return EShaderType::PixelShader;
	}
	else if (FileName.find("_GS") != std::string::npos)
	{
		return EShaderType::GeometryShader;
	}

	return EShaderType::Unknown;
}

UINT RShaderManager::GetShaderCompileFlag() const
{
#if defined(DEBUG) || defined(_DEBUG)
	return D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	return 0;
#endif
}
