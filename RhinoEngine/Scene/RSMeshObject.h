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

	void SaveMaterialsToFile();

	void SetOverridingShader(RShader* shader);

	const RAabb& GetAabb() const;

	void Draw(bool instanced = false, int instanceCount = 0);
	void DrawWithShader(RShader* shader, bool instanced = false, int instanceCount = 0);

	float GetResourceTimestamp();
private:
	RMesh*					m_Mesh;
	vector<RMaterial>		m_Materials;
	RShader*				m_OverridingShader;
	bool					m_bNeedUpdateMaterial;
};

#endif
