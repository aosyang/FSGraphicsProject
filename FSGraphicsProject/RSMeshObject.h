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
#include "RMesh.h"

#include <vector>
using namespace std;

class RSMeshObject : public RSceneObject
{
public:
	RSMeshObject();
	~RSMeshObject();

	void SetMesh(RMesh* mesh);

	int GetSubmeshCount() const;
	void SetMaterial(RMaterial* materials, int materialNum);
	RMaterial GetMaterial(int index) const;

	void Draw();
private:
	RMesh*					m_Mesh;
	vector<RMaterial>		m_Materials;
};

#endif
