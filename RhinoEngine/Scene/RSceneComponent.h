//=============================================================================
// RSceneComponentBase.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RSceneComponent.h"
#include "Core/RRuntimeTypeObject.h"

class RSceneObject;

#define DECLARE_SCENE_COMPONENT(type, base)\
		typedef base Base;\
		DECLARE_RUNTIME_TYPE(type, base)\
	public:\
		static std::unique_ptr<type> _CreateComponentUnique(RSceneObject* InOwner) { return std::unique_ptr<type>(new type(InOwner)); }\
	private:


/// Base scene component class
class RSceneComponent : public RRuntimeTypeObject
{
	DECLARE_RUNTIME_TYPE(RSceneComponent, RRuntimeTypeObject);
public:
	RSceneComponent(RSceneObject* InOwner);
	virtual ~RSceneComponent() {}

	/// Get the scene object which is owning this component
	RSceneObject* GetOwner() const;

	virtual void Update(float DeltaTime) {}

private:
	/// The scene object owning this component
	RSceneObject*	OwnerSceneObject;
};

FORCEINLINE RSceneObject* RSceneComponent::GetOwner() const
{
	return OwnerSceneObject;
}
