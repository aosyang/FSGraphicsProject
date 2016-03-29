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
}

void RVertexDeclaration::Release()
{
	for (map<string, ID3D11InputLayout*>::iterator iter = m_InputLayouts.begin();
		iter != m_InputLayouts.end(); iter++)
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
