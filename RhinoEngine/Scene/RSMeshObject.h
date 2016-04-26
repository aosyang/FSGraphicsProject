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

	SceneObjectType GetType() const { return SO_MeshObject; }

	void SetMesh(RMesh* mesh);
	RMesh* GetMesh() const;

	int GetMeshElementCount() const;
	void SetMaterial(RMaterial* materials, int materialNum);
	RMaterial GetMaterial(int index) const;

	void SaveMaterialsToFile();

	void SetOverridingShader(RShader* shader);

	RAabb GetAabb() const;
	const RAabb& GetMeshElementAabb(int index) const;

	void Draw();
	void Draw(bool instanced, int instanceCount);
	void DrawDepthPass();
	void DrawDepthPass(bool instanced, int instanceCount);
	void DrawWithShader(RShader* shader, bool instanced = false, int instanceCount = 0);

	float GetResourceTimestamp();
protected:
	void UpdateMaterialsFromResource();

	RMesh*					m_Mesh;
	vector<RMaterial>		m_Materials;
	RShader*				m_OverridingShader;
	bool					m_bNeedUpdateMaterial;
};

#endif
