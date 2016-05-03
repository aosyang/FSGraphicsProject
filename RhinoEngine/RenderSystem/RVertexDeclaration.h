//=============================================================================
// RVertexDeclaration.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RVERTEXDECLARATION_H
#define _RVERTEXDECLARATION_H

#define VERTEX_TYPE_BEGIN(v) struct v { static const char* GetTypeName() { return #v; }
#define VERTEX_TYPE_END };
#define VERTEX_TYPE_DECLARE_LEXICOGRAPHICAL_COMPARE(v)	\
		bool operator<(const v& rhs) const				\
		{												\
		return lexicographical_compare(					\
			(const float*)this,							\
			(const float*)this + sizeof(v) / 4,			\
			(const float*)&rhs,							\
			(const float*)&rhs + sizeof(v) / 4);		\
		}

enum EVertexComponent
{
	VertexComponent_Pos,
	VertexComponent_UV0,
	VertexComponent_Normal,
	VertexComponent_Tangent,
	VertexComponent_UV1,
	VertexComponent_BoneId,
	VertexComponent_BoneWeights,

	VertexComponent_Count,
};

struct ShaderInputVertex
{
	char Type[10];
	char Semantic[20];
};

enum EVertexComponentMask
{
	VCM_Pos								= 1 << VertexComponent_Pos,
	VCM_UV0								= 1 << VertexComponent_UV0,
	VCM_Normal							= 1 << VertexComponent_Normal,
	VCM_Tangent							= 1 << VertexComponent_Tangent,
	VCM_UV1								= 1 << VertexComponent_UV1,
	VCM_BoneId							= 1 << VertexComponent_BoneId,
	VCM_BoneWeights						= 1 << VertexComponent_BoneWeights,

	VCM_PosNormal						= VCM_Pos | VCM_Normal,
	VCM_PosUV0Normal					= VCM_Pos | VCM_UV0 | VCM_Normal,
	VCM_PosUV0NormalUV1					= VCM_Pos | VCM_UV0 | VCM_Normal | VCM_UV1,
	VCM_PosUV0NormalTangent				= VCM_Pos | VCM_UV0 | VCM_Normal | VCM_Tangent,
	VCM_PosUV0NormalTangentUV1			= VCM_Pos | VCM_UV0 | VCM_Normal | VCM_Tangent | VCM_UV1,
};

namespace RVertex
{
	VERTEX_TYPE_BEGIN(MESH_LOADER_VERTEX)
		RVec3 pos;
		RVec2 uv0;
		RVec2 uv1;
		RVec3 normal;
		RVec3 tangent;
		int boneId[4];
		float weight[4];
	VERTEX_TYPE_DECLARE_LEXICOGRAPHICAL_COMPARE(MESH_LOADER_VERTEX)
	VERTEX_TYPE_END

	VERTEX_TYPE_BEGIN(MESH_VERTEX)
		RVec3 pos;
		RVec2 uv0;
		RVec3 normal;
		RVec3 tangent;
		RVec2 uv1;
	VERTEX_TYPE_END

	VERTEX_TYPE_BEGIN(PRIMITIVE_VERTEX)
		RVec4	pos;
		RColor	color;
	VERTEX_TYPE_END


	VERTEX_TYPE_BEGIN(SKYBOX_VERTEX)
		RVec3 pos;
	VERTEX_TYPE_END


	VERTEX_TYPE_BEGIN(PARTICLE_VERTEX)
		RVec4 pos;
		RVec4 color;
		float rot;
		RVec4 uvScaleOffset;
	VERTEX_TYPE_END

	VERTEX_TYPE_BEGIN(FONT_VERTEX)
		RVec4	pos;
		RColor	color_fg;
		RColor	color_bg;
		RVec2	uv;
	VERTEX_TYPE_END
}

class RVertexDeclaration : public RSingleton<RVertexDeclaration>
{
public:
	RVertexDeclaration();
	~RVertexDeclaration();

	void Initialize();
	void Release();

	ID3D11InputLayout* GetInputLayout(const string& vertexTypeName) const;
	ID3D11InputLayout* GetInputLayoutByVertexComponents(int vertexComponents);
	int GetVertexStride(int vertexComponents) const;
	void CopyVertexComponents(void* out, const RVertex::MESH_LOADER_VERTEX* in, int count, int vertexComponents) const;
private:
	map<string, ID3D11InputLayout*>		m_InputLayouts;
	map<int, ID3D11InputLayout*>		m_VertexComponentInputLayouts;
};


#endif