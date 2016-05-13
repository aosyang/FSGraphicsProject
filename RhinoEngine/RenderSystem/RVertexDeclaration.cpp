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
	{ "int4",	"BLENDINDICES", },
	{ "float4", "BLENDWEIGHT", },
	{ "float3", "POSITION", },
	{ "float2", "TEXCOORD0", },
	{ "float3", "NORMAL", },
	{ "float3", "TANGENT", },
	{ "float2", "TEXCOORD1", },
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
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(meshVertDesc, 5, RMeshVertexSignature, sizeof(RMeshVertexSignature), &pInputLayout);
	m_InputLayouts.insert(make_pair(RVertex::MESH_VERTEX::GetTypeName(), pInputLayout));


	// Color primitive vertex
	D3D11_INPUT_ELEMENT_DESC primitiveVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(primitiveVertDesc, 2, RPrimitiveVertexSignature, sizeof(RPrimitiveVertexSignature), &pInputLayout);
	m_InputLayouts.insert(make_pair(RVertex::PRIMITIVE_VERTEX::GetTypeName(), pInputLayout));


	// Skybox vertex
	D3D11_INPUT_ELEMENT_DESC skyboxVertDesc[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(skyboxVertDesc, 1, RSkyboxVertexSignature, sizeof(RSkyboxVertexSignature), &pInputLayout);
	m_InputLayouts.insert(make_pair(RVertex::SKYBOX_VERTEX::GetTypeName(), pInputLayout));


	// Particle vertex
	D3D11_INPUT_ELEMENT_DESC particleVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(particleVertDesc, 4, RParticleVertexSignature, sizeof(RParticleVertexSignature), &pInputLayout);
	m_InputLayouts.insert(make_pair(RVertex::PARTICLE_VERTEX::GetTypeName(), pInputLayout));

	D3D11_INPUT_ELEMENT_DESC fontVertDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	RRenderer.D3DDevice()->CreateInputLayout(fontVertDesc, 4, RFontVertexSignature, sizeof(RFontVertexSignature), &pInputLayout);
	m_InputLayouts.insert(make_pair(RVertex::FONT_VERTEX::GetTypeName(), pInputLayout));
}

void RVertexDeclaration::Release()
{
	for (map<string, ID3D11InputLayout*>::iterator iter = m_InputLayouts.begin();
		iter != m_InputLayouts.end(); iter++)
	{
		iter->second->Release();
	}

	for (map<int, ID3D11InputLayout*>::iterator iter = m_VertexComponentInputLayouts.begin();
		 iter != m_VertexComponentInputLayouts.end(); iter++)
	{
		iter->second->Release();
	}

	m_InputLayouts.clear();
}

ID3D11InputLayout* RVertexDeclaration::GetInputLayout(const string& vertexTypeName) const
{
	map<string, ID3D11InputLayout*>::const_iterator iter = m_InputLayouts.find(vertexTypeName);
	if (iter != m_InputLayouts.end())
	{
		return iter->second;
	}

	return nullptr;
}

ID3D11InputLayout* RVertexDeclaration::GetInputLayoutByVertexComponents(int vertexComponents)
{
	string msg_buf = "Input layout: ";
	for (int i = 0; i < VertexComponent_Count; i++)
	{
		if (vertexComponents & (1 << i))
		{
			msg_buf += string(VertexSemantics[i].Semantic) + " ";
		}
	}
	msg_buf += "\n";
	OutputDebugStringA(msg_buf.data());

	map<int, ID3D11InputLayout*>::const_iterator iter = m_VertexComponentInputLayouts.find(vertexComponents);
	if (iter == m_VertexComponentInputLayouts.end())
	{
		// Create input layout based on vertex component
		static D3D11_INPUT_ELEMENT_DESC ComponentDescs[VertexComponent_Count] =
		{
			{ "BLENDINDICES",	0, DXGI_FORMAT_R32G32B32A32_SINT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
		string vertexShaderSignature = "struct INPUT_VERTEX {\n";

		for (int i = 0; i < VertexComponent_Count; i++)
		{
			if (vertexComponents & (1 << i))
			{
				string variableName = VertexSemantics[i].Semantic;
				for (UINT n = 1; n < variableName.length(); n++)
				{
					variableName[n] = tolower(variableName[n]);
				}

				vertexShaderSignature += string(VertexSemantics[i].Type) + " " + variableName + " : " + VertexSemantics[i].Semantic + ";\n";
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
			RRenderer.D3DDevice()->CreateInputLayout(desc, componentCount, pShaderCode->GetBufferPointer(), pShaderCode->GetBufferSize(), &pInputLayout);
		}
		else
		{
			OutputDebugStringA((char*)pErrorMsg->GetBufferPointer());
			OutputDebugStringA("\n");
			assert(0);
		}

		m_VertexComponentInputLayouts[vertexComponents] = pInputLayout;

		return pInputLayout;
	}
	else
	{
		return iter->second;
	}
}

int RVertexDeclaration::GetVertexStride(int vertexComponents)
{
	int stride = 0;

	if (vertexComponents & VCM_Pos)
		stride += sizeof(RVec3);
	if (vertexComponents & VCM_UV0)
		stride += sizeof(RVec2);
	if (vertexComponents & VCM_UV1)
		stride += sizeof(RVec2);
	if (vertexComponents & VCM_Normal)
		stride += sizeof(RVec3);
	if (vertexComponents & VCM_Tangent)
		stride += sizeof(RVec3);
	if (vertexComponents & VCM_BoneId)
		stride += sizeof(int) * 4;
	if (vertexComponents & VCM_BoneWeights)
		stride += sizeof(float) * 4;

	return stride;
}

void RVertexDeclaration::CopyVertexComponents(void* out, const RVertex::MESH_LOADER_VERTEX* in, int count, int vertexComponents)
{
	struct VC_Info
	{
		int size;
		int offset;
	};

	static VC_Info strides[VertexComponent_Count] =
	{
		{ GetVertexStride(VCM_BoneId),		offsetof(RVertex::MESH_LOADER_VERTEX, boneId) },
		{ GetVertexStride(VCM_BoneWeights),	offsetof(RVertex::MESH_LOADER_VERTEX, weight) },
		{ GetVertexStride(VCM_Pos),			offsetof(RVertex::MESH_LOADER_VERTEX, pos) },
		{ GetVertexStride(VCM_UV0),			offsetof(RVertex::MESH_LOADER_VERTEX, uv0) },
		{ GetVertexStride(VCM_Normal),		offsetof(RVertex::MESH_LOADER_VERTEX, normal) },
		{ GetVertexStride(VCM_Tangent),		offsetof(RVertex::MESH_LOADER_VERTEX, tangent) },
		{ GetVertexStride(VCM_UV1),			offsetof(RVertex::MESH_LOADER_VERTEX, uv1) },
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
