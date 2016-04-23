//=============================================================================
// RSceneObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSCENEOBJECT_H
#define _RSCENEOBJECT_H

class RScene;

enum SceneObjectType
{
	SO_None,
	SO_MeshObject,
	SO_Camera,
};

class RSceneObject
{
public:
	RSceneObject();
	virtual ~RSceneObject();

	void SetScene(RScene* scene)	{ m_Scene = scene; }
	RScene* GetScene() const		{ return m_Scene; }

	virtual SceneObjectType GetType() const { return SO_None; }

	const RMatrix4& GetNodeTransform() const;

	void SetTransform(const RMatrix4& transform);
	void SetPosition(const RVec3& pos);
	RVec3 GetPosition() const;

	void Translate(const RVec3& v);
	void TranslateLocal(const RVec3& v);

	virtual RAabb GetAabb() const { return RAabb::Default; }
	virtual void Draw() {}

protected:
	RMatrix4	m_NodeTransform;
	RScene*		m_Scene;
};

#endif
