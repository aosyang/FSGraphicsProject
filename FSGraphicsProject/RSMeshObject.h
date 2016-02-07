//=============================================================================
// RSMeshObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSMESHOBJECT_H
#define _RSMESHOBJECT_H

#include "Rhino.h"
#include "RSceneObject.h"
#include "RShaderManager.h"

#include <vector>
using namespace std;

struct RMaterial
{
	RShader*					Shader;
	int							TextureNum;
	ID3D11ShaderResourceView*	Textures[8];
};

class RSMeshObject : public RSceneObject
{
public:
	RSMeshObject();
	~RSMeshObject();

	void LoadFbxMesh(const char* filename, ID3D11InputLayout* inputLayout);

	int GetSubmeshCount() const;
	void SetMaterial(RMaterial* materials, int materialNum);
	RMaterial GetMaterial(int index) const;

	void Draw();
private:
	vector<RMeshElement>	m_MeshElements;
	ID3D11InputLayout*		m_InputLayout;

	vector<RMaterial>		m_Materials;
};

#endif
