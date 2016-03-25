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


class RVertexDeclaration
{
public:
	RVertexDeclaration();
	~RVertexDeclaration();

	void Initialize();
	void Release();

	ID3D11InputLayout* GetInputLayout(const string& vertexTypeName) const;

private:
	map<string, ID3D11InputLayout*>		m_InputLayouts;

};

VERTEX_TYPE_BEGIN(MESH_VERTEX)
	RVec3 pos;
	RVec2 uv0;
	RVec3 normal;
	RVec3 tangent;
	RVec2 uv1;
VERTEX_TYPE_DECLARE_LEXICOGRAPHICAL_COMPARE(MESH_VERTEX)
VERTEX_TYPE_END

VERTEX_TYPE_BEGIN(PRIMITIVE_VERTEX)
	RVec4	pos;
	RColor	color;
VERTEX_TYPE_END

#endif