//=============================================================================
// RMesh.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RMESH_H
#define _RMESH_H

#include "RShaderManager.h"
#include "IResource.h"
#include "../../Shaders/ConstBufferVS.h"	// MAX_BONE_COUNT

struct RMaterial
{
	RShader*					Shader;
	int							TextureNum;
	RTexture*					Textures[8];
};

struct BoneMatrices
{
	RMatrix4 boneMatrix[MAX_BONE_COUNT];
};

class RMesh : public RBaseResource
{
public:
	RMesh(string path);
	RMesh(string path, const vector<RMeshElement> meshElements, const vector<RMaterial>& materials);
	RMesh(string path, RMeshElement* meshElements, int numElement, RMaterial* materials, int numMaterial);
	~RMesh();

	RMaterial GetMaterial(int index) const;
	vector<RMaterial>& GetMaterials();

	void SetInputLayout(ID3D11InputLayout* inputLayout);
	ID3D11InputLayout* GetInputLayout() const;
	vector<RMeshElement>& GetMeshElements();
	int GetMeshElementCount() const;

	void SetMeshElements(RMeshElement* meshElements, int numElement);
	void SetMaterials(RMaterial* materials, int numMaterial);

	void SetAabb(const RAabb& aabb);
	const RAabb& GetAabb() const;
	const RAabb& GetMeshElementAabb(int index) const;

	void SetAnimation(RAnimation* anim);
	RAnimation* GetAnimation() const;

	void SetBoneInitInvMatrices(BoneMatrices* bonePoses);
	const RMatrix4& GetBoneInitInvMatrices(int index) const { return m_BoneInitInvMatrices->boneMatrix[index]; }

	void SetResourceTimestamp(float time);
	float GetResourceTimestamp();
private:
	vector<RMeshElement>	m_MeshElements;
	ID3D11InputLayout*		m_InputLayout;

	vector<RMaterial>		m_Materials;
	RAabb					m_Aabb;
	float					m_LoadingFinishTime;

	RAnimation*				m_Animation;
	BoneMatrices*			m_BoneInitInvMatrices;
};

#endif
