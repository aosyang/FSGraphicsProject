//=============================================================================
// RSceneObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RSCENEOBJECT_H
#define _RSCENEOBJECT_H

class RScene;
class RSceneComponentBase;

enum class ESceneObjectType : UINT8
{
	None,
	MeshObject,
	Camera,
};

#define DECLARE_SCENE_OBJECT(base)\
	typedef base Base; friend class RScene;\
	\
	static int _RuntimeTypeId;\
	public:\
	static int _StaticGetRuntimeTypeId() { if (_RuntimeTypeId == 0) { _RuntimeTypeId = RSceneObject::MakeUniqueRuntimeTypeId(); } return _RuntimeTypeId; }\
	virtual int GetRuntimeTypeId() override { return _StaticGetRuntimeTypeId(); }\
	private:

#define IMPLEMENT_SCENE_OBJECT(type)\
	int type::_RuntimeTypeId = 0

/// Base object that can be placed in a scene
class RSceneObject
{
	friend class RScene;
public:
	virtual void Release();

	virtual int GetRuntimeTypeId() { return 0; }

	/// Set name of scene object
	void SetName(const string& name)		{ m_Name = name; }

	/// Get name of scene object
	const string& GetName() const			{ return m_Name; }

	/// Get the scene which this object belongs to
	RScene* GetScene() const				{ return m_Scene; }

	virtual ESceneObjectType GetType() const { return ESceneObjectType::None; }
	virtual RSceneObject* Clone() const		{ return nullptr; }

	RTransform* GetTransform();
	const RMatrix4& GetTransformMatrix();

	/// Set matrix as transform of scene object
	void SetTransform(const RMatrix4& transform);

	/// Set transform from position, rotation and scale
	void SetTransform(const RVec3& InPosition, const RQuat& InRotation, const RVec3& InScale = RVec3(1, 1, 1));

	/// Set transform from another transform
	void SetTransform(const RTransform& InTransform);

	/// Set rotation by quaternion
	void SetRotation(const RQuat& quat);
	
	/// Set position of scene object
	void SetPosition(const RVec3& pos);

	/// Get position of scene object
	RVec3 GetPosition() const;

	RVec3 GetWorldPosition();

	/// Get the forward vector of scene object
	RVec3 GetForwardVector() const;

	/// Get the right vector of scene object
	RVec3 GetRightVector() const;

	/// Get the up vector of scene object
	RVec3 GetUpVector() const;

	void SetScale(const RVec3& scale);

	const RVec3& GetScale() const;

	/// Rotate scene object towards given target
	void LookAt(const RVec3& target, const RVec3& world_up = RVec3(0, 1, 0));

	/// Move scene object in world space
	void Translate(const RVec3& v, ETransformSpace Space);

	/// Attach to another object
	void AttachTo(RSceneObject* Parent);

	/// Detach from parent object
	void DetachFromParent();

	/// Get world space AABB for scene object
	virtual const RAabb& GetAabb() { return RAabb::Default; }
	virtual void Draw() {}
	virtual void DrawDepthPass() {}

	virtual void Update();

	/// Create a new component and add to this scene object
	template<typename T>
	T* AddNewComponent();

	/// Set script string with function name and parameters to be invoked
	/// example: 'RotateObject 1, 0, 0 50' - rotate object around axis [1, 0, 0] by 50 degree
	void SetScript(const string& script)	{ m_Script = script; }

	/// Get script string
	const string& GetScript() const			{ return m_Script; }

	/// Get split script strings in array form
	const vector<string>& GetParsedScript();

protected:
	RSceneObject(RScene* InScene);
	virtual ~RSceneObject();

	/// Update all components on this scene object
	void UpdateComponents();

	/// Create a unique runtime type id for scene object types
	static int MakeUniqueRuntimeTypeId()
	{
		return ++NextUniqueRuntimeTypeId;
	}

protected:
	string			m_Name;
	RTransform		m_NodeTransform;
	RScene*			m_Scene;
	string			m_Script;
	vector<string>	m_ParsedScript;

	/// Components of this scene object
	vector<RSceneComponentBase*>	SceneComponents;

	static int NextUniqueRuntimeTypeId;
};


template<typename T>
FORCEINLINE T* RSceneObject::AddNewComponent()
{
	T* NewComponent = T::Create(this);
	SceneComponents.push_back(NewComponent);

	return NewComponent;
}

FORCEINLINE void RSceneObject::SetTransform(const RVec3& InPosition, const RQuat& InRotation, const RVec3& InScale /*= RVec3(1, 1, 1)*/)
{
	m_NodeTransform.SetPosition(InPosition);
	m_NodeTransform.SetRotation(InRotation);
	m_NodeTransform.SetScale(InScale);
}

FORCEINLINE void RSceneObject::SetTransform(const RTransform& InTransform)
{
	m_NodeTransform = InTransform;
}

#endif
