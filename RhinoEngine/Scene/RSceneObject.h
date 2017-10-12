//=============================================================================
// RSceneObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSCENEOBJECT_H
#define _RSCENEOBJECT_H

class RScene;
class RSceneComponentBase;

enum SceneObjectType
{
	SO_None,
	SO_MeshObject,
	SO_Camera,
};

/// Base object that can be placed in a scene
class RSceneObject
{
public:
	RSceneObject();
	virtual ~RSceneObject();

	/// Set name of scene object
	void SetName(const string& name)	{ m_Name = name; }

	/// Get name of scene object
	const string& GetName() const		{ return m_Name; }

	void SetScene(RScene* scene)	{ m_Scene = scene; }
	RScene* GetScene() const		{ return m_Scene; }

	virtual SceneObjectType GetType() const { return SO_None; }
	virtual RSceneObject* Clone() const { return nullptr; }

	const RMatrix4& GetNodeTransform() const;

	/// Set matrix as transform of scene object
	void SetTransform(const RMatrix4& transform);

	/// Set matrix as rotation of scene object (only 3x3 from matrix is used)
	void SetRotation(const RMatrix4& rot);
	
	/// Set position of scene object
	void SetPosition(const RVec3& pos);

	/// Get position of scene object
	RVec3 GetPosition() const;

	/// Rotate scene object towards given target
	void LookAt(const RVec3& target, const RVec3& world_up = RVec3(0, 1, 0));

	/// Move scene object in world space
	void Translate(const RVec3& v);

	/// Move scene object in local space
	void TranslateLocal(const RVec3& v);

	/// Get world space AABB for scene object
	virtual const RAabb& GetAabb() { return RAabb::Default; }
	virtual void Draw() {}
	virtual void DrawDepthPass() {}

	/// Create a new component and add to this scene object
	template<typename T>
	T* AddSceneComponent();

	/// Set script string with function name and parameters to be invoked
	/// example: 'RotateObject 1, 0, 0 50' - rotate object around axis [1, 0, 0] by 50 degree
	void SetScript(const string& script)	{ m_Script = script; }

	/// Get script string
	const string& GetScript() const			{ return m_Script; }

	/// Get split script strings in array form
	const vector<string>& GetParsedScript();

protected:
	/// Update all components on this scene object
	void UpdateComponents();

protected:
	string			m_Name;
	RMatrix4		m_NodeTransform;
	RScene*			m_Scene;
	string			m_Script;
	vector<string>	m_ParsedScript;

	/// Components of this scene object
	vector<RSceneComponentBase*>	SceneComponents;
};


template<typename T>
FORCEINLINE T* RSceneObject::AddSceneComponent()
{
	T* NewComponent = T::Create(this);
	SceneComponents.push_back(NewComponent);

	return NewComponent;
}


#endif
