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
	VertexComponent_BoneId,
	VertexComponent_BoneWeights,
	VertexComponent_Pos,
	VertexComponent_UV0,
	VertexComponent_Normal,
	VertexComponent_Tangent,
	VertexComponent_UV1,

	VertexComponent_Count,
};

struct ShaderInputVertex
{
	char Type[10];
	char Semantic[20];
};

enum EVertexComponentMask
{
	VCM_BoneId							= 1 << VertexComponent_BoneId,
	VCM_BoneWeights						= 1 << VertexComponent_BoneWeights,
	VCM_Pos								= 1 << VertexComponent_Pos,
	VCM_UV0								= 1 << VertexComponent_UV0,
	VCM_Normal							= 1 << VertexComponent_Normal,
	VCM_Tangent							= 1 << VertexComponent_Tangent,
	VCM_UV1								= 1 << VertexComponent_UV1,
};

enum EVertexComponents
{
	VC_PosNormal						= VCM_Pos | VCM_Normal,
	VC_PosUV0Normal						= VCM_Pos | VCM_UV0 | VCM_Normal,
	VC_PosUV0NormalUV1					= VCM_Pos | VCM_UV0 | VCM_Normal | VCM_UV1,
	VC_PosUV0NormalTangent				= VCM_Pos | VCM_UV0 | VCM_Normal | VCM_Tangent,
	VC_PosUV0NormalTangentUV1			= VCM_Pos | VCM_UV0 | VCM_Normal | VCM_Tangent | VCM_UV1,
};

namespace RVertex
{
	struct Vec2Data
	{
		float x, y;

		Vec2Data() {}
		Vec2Data(float _x, float _y)
			: x(_x), y(_y)
		{}
	};

	struct Vec3Data
	{
		float x, y, z;

		Vec3Data() {}
		Vec3Data(float _x, float _y, float _z)
			: x(_x), y(_y), z(_z)
		{}
	};

	struct Vec4Data
	{
		float x, y, z, w;

		Vec4Data() {}
		Vec4Data(float _x, float _y, float _z, float _w)
			: x(_x), y(_y), z(_z), w(_w)
		{}
		Vec4Data(const RVec4& v)
			: x(v.x), y(v.y), z(v.z), w(v.w)
		{}

		float& operator[](long index)
		{
			return *(&x + index);
		}
	};

	VERTEX_TYPE_BEGIN(MESH_LOADER_VERTEX)
		Vec3Data pos;
		Vec2Data uv0;
		Vec2Data uv1;
		Vec3Data normal;
		Vec3Data tangent;
		int boneId[4];
		Vec4Data weight;
	VERTEX_TYPE_DECLARE_LEXICOGRAPHICAL_COMPARE(MESH_LOADER_VERTEX)
	VERTEX_TYPE_END

	VERTEX_TYPE_BEGIN(MESH_VERTEX)
		Vec3Data pos;
		Vec2Data uv0;
		Vec3Data normal;
		Vec3Data tangent;
		Vec2Data uv1;
	VERTEX_TYPE_END

	VERTEX_TYPE_BEGIN(PRIMITIVE_VERTEX)
		Vec4Data	pos;
		RColor		color;
	VERTEX_TYPE_END


	VERTEX_TYPE_BEGIN(SKYBOX_VERTEX)
		Vec3Data pos;
	VERTEX_TYPE_END


	VERTEX_TYPE_BEGIN(PARTICLE_VERTEX)
		Vec4Data pos;
		Vec4Data color;
		float rot;
		Vec4Data uvScaleOffset;
	VERTEX_TYPE_END

	VERTEX_TYPE_BEGIN(FONT_VERTEX)
		Vec4Data	pos;
		RColor		color_fg;
		RColor		color_bg;
		Vec2Data	uv;
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
	static string GetVertexComponentsString(int vertexComponents);
	static int GetVertexStride(int vertexComponents);
	static void CopyVertexComponents(void* out, const RVertex::MESH_LOADER_VERTEX* in, int count, int vertexComponents);
private:
	map<string, ID3D11InputLayout*>		m_InputLayouts;
	map<int, ID3D11InputLayout*>		m_VertexComponentInputLayouts;
};


#endif