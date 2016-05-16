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

	void SetName(const string& name)	{ m_Name = name; }
	string GetName() const				{ return m_Name; }

	void SetScene(RScene* scene)	{ m_Scene = scene; }
	RScene* GetScene() const		{ return m_Scene; }

	virtual SceneObjectType GetType() const { return SO_None; }
	virtual RSceneObject* Clone() const { return nullptr; }

	const RMatrix4& GetNodeTransform() const;

	void SetTransform(const RMatrix4& transform);
	void SetRotation(const RMatrix4& rot);
	void SetPosition(const RVec3& pos);
	RVec3 GetPosition() const;

	void LookAt(const RVec3 target);

	void Translate(const RVec3& v);
	void TranslateLocal(const RVec3& v);

	virtual RAabb GetAabb() const { return RAabb::Default; }
	virtual void Draw() {}
	virtual void DrawDepthPass() {}

protected:
	string		m_Name;
	RMatrix4	m_NodeTransform;
	RScene*		m_Scene;
};

#endif
