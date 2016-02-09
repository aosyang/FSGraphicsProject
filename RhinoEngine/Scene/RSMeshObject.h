//=============================================================================
// RSMeshObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSMESHOBJECT_H
#define _RSMESHOBJECT_H

#include "RSceneObject.h"

class RSMeshObject : public RSceneObject
{
public:
	RSMeshObject();
	~RSMeshObject();

	void SetMesh(RMesh* mesh);

	int GetSubmeshCount() const;
	void SetMaterial(RMaterial* materials, int materialNum);
	RMaterial GetMaterial(int index) const;

	void Draw(bool instanced = false, int instanceCount = 0);
private:
	RMesh*					m_Mesh;
	vector<RMaterial>		m_Materials;
};

#endif
