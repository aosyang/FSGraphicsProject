//=============================================================================
// RSMeshObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSMESHOBJECT_H
#define _RSMESHOBJECT_H

#include "RSceneObject.h"

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

class RSMeshObject : public RSceneObject
{
public:
	RSMeshObject();
	~RSMeshObject();

	SceneObjectType GetType() const { return SO_MeshObject; }
	RSceneObject* Clone() const;

	void SetMesh(RMesh* mesh);
	RMesh* GetMesh() const;

	int GetMeshElementCount() const;
	void SetMaterial(RMaterial* materials, int materialNum);
	RMaterial* GetMaterial(int index);

	void SaveMaterialsToFile();
	void SerializeMaterialsToXML(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* elem_mat);

	void SetOverridingShader(RShader* shader, int features = -1);

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
	int						m_OverridingShaderFeatures;
	bool					m_bNeedUpdateMaterial;
};

#endif
