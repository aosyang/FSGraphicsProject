//=============================================================================
// RSMeshObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RSceneObject.h"

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

class RSMeshObject : public RSceneObject
{
	DECLARE_SCENE_OBJECT(RSMeshObject, RSceneObject);
public:
	RSceneObject* Clone() override;

	void SetMesh(RMesh* mesh);
	RMesh* GetMesh() const;

	int GetMeshElementCount() const;
	void SetMaterial(RMaterial* materials, int materialNum);
	RMaterial* GetMaterial(int index);

	void SaveMaterialsToFile();
	void SerializeMaterialsToXML(tinyxml2::XMLDocument* doc, tinyxml2::XMLElement* elem_mat);

	void SetOverridingShader(RShader* shader, int features = -1);

	const RAabb& GetAabb() override;
	const RAabb& GetMeshElementAabb(int index) const;

	// Overrides RSceneObject render methods
	virtual void Draw() override;
	virtual void DrawDepthPass() override;

	void Draw(bool instanced, int instanceCount);
	void DrawDepthPass(bool instanced, int instanceCount);
	void DrawWithShader(RShader* shader, bool instanced = false, int instanceCount = 0);

	float GetResourceTimestamp();
protected:
	RSMeshObject(const RConstructingParams& Params);
	~RSMeshObject();

	/// Use default materials defined in mesh resource
	void SetupMaterialsFromMeshResource();

	RMesh*					m_Mesh;
	vector<RMaterial>		m_Materials;
	RAabb					m_MeshAABB;
	RShader*				m_OverridingShader;
	int						m_OverridingShaderFeatures;
	bool					m_bNeedUpdateMaterial;
};

