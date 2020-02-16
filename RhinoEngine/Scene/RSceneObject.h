//=============================================================================
// RSceneObject.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RSceneComponent.h"
#include "Core/RRuntimeTypeObject.h"

class RScene;

#define DECLARE_SCENE_OBJECT(type, base)\
		typedef base Base; friend class RScene;\
		DECLARE_RUNTIME_TYPE(type, base)

// Note: Implementation is not being used currently
#define IMPLEMENT_SCENE_OBJECT(type)


/// Object creation flags
const int CF_InternalObject		= 1 << 0;		// Object will not be manipulated by user directly
const int CF_NoSerialization	= 1 << 1;		// Object will not be serialized

/// Scene object constructing parameters
struct RConstructingParams
{
	explicit RConstructingParams(const RConstructingParams& cpy)
		: Scene(cpy.Scene)
		, Flags(cpy.Flags)
	{
	}

	explicit RConstructingParams(RScene* InScene = nullptr, int InFlags = 0)
		: Scene(InScene)
		, Flags(InFlags)
	{
	}

	RScene* Scene;
	int		Flags;
};

/// Base object that can be placed in a scene
class RSceneObject : public RRuntimeTypeObject
{
	friend class RScene;
	DECLARE_RUNTIME_TYPE(RSceneObject, RRuntimeTypeObject);
public:
	virtual void Release();

	/// Set name of scene object
	void SetName(const std::string& name)		{ m_Name = name; }

	/// Get name of scene object
	const std::string& GetName() const			{ return m_Name; }

	/// Get the scene which this object belongs to
	RScene* GetScene() const				{ return m_Scene; }

	/// Make a copy of scene object
	virtual RSceneObject* Clone() 			{ return nullptr; }

	/// Destroy the object
	void Destroy();

	RTransform* GetTransform();
	const RMatrix4& GetTransformMatrix() const;

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

	/// Get the rotation of scene object
	const RQuat GetRotation() const;

	/// Set the scale of scene object
	void SetScale(const RVec3& scale);

	/// Get the scale of scene object
	const RVec3& GetScale() const;

	/// Rotate scene object towards given target
	void LookAt(const RVec3& target, const RVec3& world_up = RVec3(0, 1, 0));

	/// Move scene object in world space
	void Translate(const RVec3& v, ETransformSpace Space);

	/// Attach to another object
	void AttachTo(RSceneObject* Parent);

	/// Detach from parent object
	void DetachFromParent();

	void IncreaseInternalTransformUpdateCounter();
	void DecreaseInternalTransformUpdateCounter();

	/// Get world space AABB for scene object
	const RAabb& GetAabb();
	virtual void Draw() {}
	virtual void DrawDepthPass() {}

	virtual void Update(float DeltaTime);

	/// Set the visibility of scene object
	void SetVisible(bool bVisible);

	/// Get the visibility of scene object
	bool IsVisible() const;

	/// Check if scene object has flags
	bool HasFlags(int FlagMasks) const;

	/// Create a new component and add to this scene object
	template<typename T>
	T* AddNewComponent();

	/// Find a component by class
	template<typename T>
	T* FindComponent() const;

	/// Find a component by class or add it if it doesn't exist
	template<typename T>
	T* FindOrAddComponent();

	/// Set script string with function name and parameters to be invoked
	/// example: 'RotateObject 1, 0, 0 50' - rotate object around axis [1, 0, 0] by 50 degree
	void SetScript(const std::string& script)	{ m_Script = script; }

	/// Get script string
	const std::string& GetScript() const			{ return m_Script; }

	/// Get split script strings in array form
	const std::vector<std::string>& GetParsedScript();

protected:
	RSceneObject(const RConstructingParams& Params);
	virtual ~RSceneObject();

	/// Calculate bounds of the object
	virtual void CalculateBounds();

	/// Update all components on this scene object
	void UpdateComponents(float DeltaTime);

	void NotifyTransformModified();

	/// Notify derived classes about transform being changed by setting position, rotation or scale directly
	virtual void OnTransformModified();

protected:
	std::string			m_Name;
	RTransform		m_NodeTransform;
	RScene*			m_Scene;
	bool			m_bVisible;
	std::string			m_Script;
	std::vector<std::string>	m_ParsedScript;

	bool			bTransformModified;

	int				m_Flags;
	RAabb			Bounds;

	/// Number of frame in which the bounding box get updated
	UINT64			BoundsUpdateFrame;

	/// If > 0, changing transform will not result in calling OnTransformModified
	int				InternalTransformUpdateCounter;

	/// Components of this scene object
	std::vector<std::unique_ptr<RSceneComponent>>	SceneComponents;
};

/// Helper class for updating transform of scene objects without triggering OnTransformModified
class RScopeInternalTransformUpdate
{
public:
	RScopeInternalTransformUpdate(RSceneObject* InSceneObject);
	~RScopeInternalTransformUpdate();

private:
	RSceneObject* SceneObject;
};


template<typename T>
FORCEINLINE T* RSceneObject::AddNewComponent()
{
	SceneComponents.push_back(move(T::_CreateComponentUnique(this)));

	RSceneComponent* NewComponent = SceneComponents.back().get();
	NewComponent->NotifyComponentAdded();

	return static_cast<T*>(NewComponent);
}

template<typename T>
FORCEINLINE T* RSceneObject::FindComponent() const
{
	for (auto& SceneComponent : SceneComponents)
	{
		if (SceneComponent->CanCastTo<T>())
		{
			return static_cast<T*>(SceneComponent.get());
		}
	}

	return nullptr;
}

template<typename T>
FORCEINLINE T* RSceneObject::FindOrAddComponent()
{
	T* Comp = FindComponent<T>();
	if (!Comp)
	{
		Comp = AddNewComponent<T>();
	}

	return Comp;
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

FORCEINLINE void RSceneObject::SetVisible(bool bVisible)
{
	m_bVisible = bVisible;
}

FORCEINLINE bool RSceneObject::IsVisible() const
{
	return m_bVisible;
}

FORCEINLINE bool RSceneObject::HasFlags(int FlagMasks) const
{
	return (m_Flags & FlagMasks) != 0;
}
