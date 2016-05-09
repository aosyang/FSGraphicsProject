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

	void Serialize(RSerializer& serializer);
};

struct BoneMatrices
{
	RMatrix4 boneMatrix[MAX_BONE_COUNT];
};

class RMesh : public RBaseResource
{
public:
	RMesh(string path);
	RMesh(string path, const vector<RMeshElement>& meshElements, const vector<RMaterial>& materials);
	RMesh(string path, RMeshElement* meshElements, int numElement, RMaterial* materials, int numMaterial);
	~RMesh();

	void Serialize(RSerializer& serializer);

	RMaterial GetMaterial(int index) const;
	vector<RMaterial>& GetMaterials();

	vector<RMeshElement>& GetMeshElements();
	int GetMeshElementCount() const;

	void SetMeshElements(RMeshElement* meshElements, UINT numElement);
	void SetMaterials(RMaterial* materials, UINT numMaterial);

	void UpdateAabb();
	const RAabb& GetLocalSpaceAabb() const;
	const RAabb& GetMeshElementAabb(int index) const;

	void SetAnimation(RAnimation* anim);
	RAnimation* GetAnimation() const;

	void SetBoneInitInvMatrices(vector<RMatrix4>& bonePoses);
	const RMatrix4& GetBoneInitInvMatrices(int index) const { return m_BoneInitInvMatrices[index]; }

	void SetBoneNameList(const vector<string>& boneNameList);
	const string& GetBoneName(int boneId) const;
	int GetBoneCount() const;

	void CacheAnimation(RAnimation* anim);
	int GetCachedAnimationNodeId(RAnimation* anim, int boneId);

	void SetResourceTimestamp(float time);
	float GetResourceTimestamp();
private:
	vector<RMeshElement>	m_MeshElements;

	vector<RMaterial>		m_Materials;
	RAabb					m_Aabb;
	float					m_LoadingFinishTime;

	RAnimation*				m_Animation;
	vector<RMatrix4>		m_BoneInitInvMatrices;
	vector<string>			m_BoneIdToName;
	map<RAnimation*, vector<int>>
							m_AnimationNodeCache;
};

#endif
