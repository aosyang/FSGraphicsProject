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
	void SetMaterials(const std::vector<RMaterial*>& materials);

	/// Get a material at index
	RMaterial* GetMaterial(int index);

	/// Get number of materials
	int GetNumMaterials() const;

	/// Save current materials to disk and use them as default materials for the mesh
	void SaveMaterialsToDiskAsDefaults();

	void SerializeXmlMaterials_Load(tinyxml2::XMLElement* XmlElemMaterial);

	/// Save current materials to an XML document
	void SerializeXmlMaterials_Save(tinyxml2::XMLDocument* XmlDoc, tinyxml2::XMLElement* XmlElemMaterial);

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
	std::vector<RMaterial*>	m_Materials;
	RAabb					m_MeshAABB;
	RShader*				m_OverridingShader;
	int						m_OverridingShaderFeatures;
	bool					m_bNeedUpdateMaterial;
};

FORCEINLINE int RSMeshObject::GetNumMaterials() const
{
	return (int)m_Materials.size();
}
