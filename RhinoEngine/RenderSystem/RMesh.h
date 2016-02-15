//=============================================================================
// RMesh.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RMESH_H
#define _RMESH_H

#include "RShaderManager.h"
#include "IResource.h"

struct RMaterial
{
	RShader*					Shader;
	int							TextureNum;
	ID3D11ShaderResourceView*	Textures[8];
};

class RMesh : public IResource
{
public:
	RMesh(ID3D11InputLayout* inputLayout);
	RMesh(const vector<RMeshElement> meshElements, const vector<RMaterial>& materials, ID3D11InputLayout* inputLayout);
	RMesh(RMeshElement* meshElements, int numElement, RMaterial* materials, int numMaterial, ID3D11InputLayout* inputLayout);
	~RMesh();

	RMaterial GetMaterial(int index) const;
	vector<RMaterial>& GetMaterials();
	int GetSubmeshCount() const;

	ID3D11InputLayout* GetInputLayout() const;
	vector<RMeshElement>& GetMeshElements();

	void SetMeshElements(RMeshElement* meshElements, int numElement);
	void SetResourceTimestamp(float time);
	float GetResourceTimestamp();
private:
	vector<RMeshElement>	m_MeshElements;
	ID3D11InputLayout*		m_InputLayout;

	vector<RMaterial>		m_Materials;
	float					m_LoadingFinishTime;
};

#endif
