//=============================================================================
// RVertexDeclaration.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RVertexDeclaration.h"

#include "RMeshVertexSignature.csh"
#include "RPrimitiveVertexSignature.csh"
#include "RSkyboxVertexSignature.csh"
#include "RParticleVertexSignature.csh"
#include "RFontVertexSignature.csh"

ShaderInputVertex VertexSemantics[] =
{
	{ "int4",	"BLENDINDICES",	},
	{ "float4", "BLENDWEIGHT",	},
	{ "float3", "POSITION",		},
	{ "float3", "NORMAL",		},
	{ "float3", "TANGENT",		},
	{ "float2", "TEXCOORD0",	},
	{ "float2", "TEXCOORD1",	},
};


RVertexDeclaration::RVertexDeclaration()
{
}


RVertexDeclaration::~RVertexDeclaration()
{
}

void RVertexDeclaration::Initialize()
{
	ID3D11InputLayout* pInputLayout;

	// Mesh vertex
	D3D11_INPUT_ELEMENT_DESC meshVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	GRenderer.D3DDevice()->CreateInputLayout(meshVertDesc, 5, RMeshVertexSignature, sizeof(RMeshVertexSignature), &pInputLayout);
	m_InputLayouts.insert(std::make_pair(RVertexType::Mesh::GetVertexTypeName(), pInputLayout));


	// Color primitive vertex
	D3D11_INPUT_ELEMENT_DESC primitiveVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	GRenderer.D3DDevice()->CreateInputLayout(primitiveVertDesc, 2, RPrimitiveVertexSignature, sizeof(RPrimitiveVertexSignature), &pInputLayout);
	m_InputLayouts.insert(std::make_pair(RVertexType::PositionColor::GetVertexTypeName(), pInputLayout));


	// Skybox vertex
	D3D11_INPUT_ELEMENT_DESC skyboxVertDesc[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	GRenderer.D3DDevice()->CreateInputLayout(skyboxVertDesc, 1, RSkyboxVertexSignature, sizeof(RSkyboxVertexSignature), &pInputLayout);
	m_InputLayouts.insert(std::make_pair(RVertexType::Position::GetVertexTypeName(), pInputLayout));


	// Particle vertex
	D3D11_INPUT_ELEMENT_DESC particleVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	GRenderer.D3DDevice()->CreateInputLayout(particleVertDesc, 4, RParticleVertexSignature, sizeof(RParticleVertexSignature), &pInputLayout);
	m_InputLayouts.insert(std::make_pair(RVertexType::Particle::GetVertexTypeName(), pInputLayout));

	D3D11_INPUT_ELEMENT_DESC fontVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	GRenderer.D3DDevice()->CreateInputLayout(fontVertDesc, 4, RFontVertexSignature, sizeof(RFontVertexSignature), &pInputLayout);
	m_InputLayouts.insert(std::make_pair(RVertexType::Font::GetVertexTypeName(), pInputLayout));
}

void RVertexDeclaration::Release()
{
	for (const auto& Iter : m_InputLayouts)
	{
		Iter.second->Release();
	}

	for (const auto& Iter : m_VertexComponentInputLayouts)
	{
		Iter.second->Release();
	}

	m_InputLayouts.clear();
}

ID3D11InputLayout* RVertexDeclaration::GetInputLayout(const std::string& vertexTypeName) const
{
	auto Iter = m_InputLayouts.find(vertexTypeName);
	if (Iter != m_InputLayouts.end())
	{
		return Iter->second;
	}

	return nullptr;
}

ID3D11InputLayout* RVertexDeclaration::GetInputLayoutByVertexComponents(int vertexComponents)
{
#if 0
	// Log vertex components
	{
		std::string msg_buf = "Input layout: ";
		msg_buf += GetVertexComponentsString(vertexComponents);
		msg_buf += "\n";
		RLog(msg_buf.data());
	}
#endif

	auto Iter = m_VertexComponentInputLayouts.find(vertexComponents);
	if (Iter == m_VertexComponentInputLayouts.end())
	{
		// Create input layout based on vertex component
		static D3D11_INPUT_ELEMENT_DESC ComponentDescs[VertexComponent_Count] =
		{
			{ "BLENDINDICES",	0, DXGI_FORMAT_R32G32B32A32_SINT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",		1, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ID3D11InputLayout* pInputLayout;
		int componentCount = 0;
		D3D11_INPUT_ELEMENT_DESC desc[VertexComponent_Count];
		for (int i = 0; i < VertexComponent_Count; i++)
		{
			if (vertexComponents & (1 << i))
			{
				desc[componentCount] = ComponentDescs[i];
				componentCount++;
			}
		}

		// Create shader
		std::string vertexShaderSignature = "struct INPUT_VERTEX {\n";

		for (int i = 0; i < VertexComponent_Count; i++)
		{
			if (vertexComponents & (1 << i))
			{
				std::string variableName = VertexSemantics[i].Semantic;
				for (UINT n = 1; n < variableName.length(); n++)
				{
					variableName[n] = tolower(variableName[n]);
				}

				vertexShaderSignature += std::string(VertexSemantics[i].Type) + " " + variableName + " : " + VertexSemantics[i].Semantic + ";\n";
			}
		}

		vertexShaderSignature +=
			"};\n"
			"float4 main(INPUT_VERTEX Input) : SV_POSITION { return float4(0, 0, 0, 0); }\n";

		ID3DBlob* pShaderCode = nullptr;
		ID3DBlob* pErrorMsg = nullptr;

		char filename[1024];
		sprintf_s(filename, 1024, "VS_Signature_%d.hlsl", vertexComponents);

		if (SUCCEEDED(D3DCompile(vertexShaderSignature.data(), vertexShaderSignature.size(), filename, NULL, NULL, "main", "vs_4_0", 0, 0, &pShaderCode, &pErrorMsg)))
		{
			GRenderer.D3DDevice()->CreateInputLayout(desc, componentCount, pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), &pInputLayout);
		}
		else
		{
			RLog("%s\n", (char*)pErrorMsg->GetBufferPointer());
			assert(0);
		}

		m_VertexComponentInputLayouts[vertexComponents] = pInputLayout;

		return pInputLayout;
	}
	else
	{
		return Iter->second;
	}
}

std::string RVertexDeclaration::GetVertexComponentsString(int vertexComponents)
{
	std::string str;

	for (int i = 0; i < VertexComponent_Count; i++)
	{
		if (vertexComponents & (1 << i))
		{
			str += std::string(VertexSemantics[i].Semantic) + " ";
		}
	}

	return str;
}

int RVertexDeclaration::GetVertexStride(int vertexComponents)
{
	int stride = 0;

	if (vertexComponents & VCM_Pos)
		stride += sizeof(RVertexType::Vec3Data);
	if (vertexComponents & VCM_UV0)
		stride += sizeof(RVertexType::Vec2Data);
	if (vertexComponents & VCM_UV1)
		stride += sizeof(RVertexType::Vec2Data);
	if (vertexComponents & VCM_Normal)
		stride += sizeof(RVertexType::Vec3Data);
	if (vertexComponents & VCM_Tangent)
		stride += sizeof(RVertexType::Vec3Data);
	if (vertexComponents & VCM_BoneId)
		stride += sizeof(int) * 4;
	if (vertexComponents & VCM_BoneWeights)
		stride += sizeof(float) * 4;

	return stride;
}

void RVertexDeclaration::CopyVertexComponents(void* out, const RVertexType::MeshLoader* in, int count, int vertexComponents)
{
	struct VC_Info
	{
		int size;
		int offset;
	};

	static VC_Info strides[VertexComponent_Count] =
	{
		{ GetVertexStride(VCM_BoneId),		offsetof(RVertexType::MeshLoader, boneId) },
		{ GetVertexStride(VCM_BoneWeights),	offsetof(RVertexType::MeshLoader, weight) },
		{ GetVertexStride(VCM_Pos),			offsetof(RVertexType::MeshLoader, pos) },
		{ GetVertexStride(VCM_UV0),			offsetof(RVertexType::MeshLoader, uv0) },
		{ GetVertexStride(VCM_Normal),		offsetof(RVertexType::MeshLoader, normal) },
		{ GetVertexStride(VCM_Tangent),		offsetof(RVertexType::MeshLoader, tangent) },
		{ GetVertexStride(VCM_UV1),			offsetof(RVertexType::MeshLoader, uv1) },
	};

	int offset = 0;
	for (int i = 0; i < count; i++)
	{
		for (int n = 0; n < VertexComponent_Count; n++)
		{
			if ((1 << n) & vertexComponents)
			{
				memcpy((BYTE*)out + offset, (BYTE*)&in[i] + strides[n].offset, strides[n].size);
				offset += strides[n].size;
			}
		}
	}
}
