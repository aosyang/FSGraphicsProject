//=============================================================================
// RSceneComponentBase.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "ISceneComponent.h"

class RSceneObject;

#define DECLARE_SCENE_COMPONENT(type, base)\
		typedef base Base;\
	public:\
		static unique_ptr<type> _CreateComponentUnique(RSceneObject* InOwner) { return unique_ptr<type>(new type(InOwner)); }\
	private:


/// Base scene component class
class RSceneComponentBase : public ISceneComponent
{
public:
	RSceneComponentBase(RSceneObject* InOwner);

	/// Get the scene object which is owning this component
	RSceneObject* GetOwner() const;

	virtual void Update(float DeltaTime) override {}

private:
	/// The scene object owning this component
	RSceneObject*	OwnerSceneObject;
};

FORCEINLINE RSceneObject* RSceneComponentBase::GetOwner() const
{
	return OwnerSceneObject;
}
