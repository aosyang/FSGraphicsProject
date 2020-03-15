//=============================================================================
// RSceneComponentBase.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RSceneComponent.h"
#include "Core/RRuntimeTypeObject.h"

class RSceneObject;
class RSceneComponent;

namespace tinyxml2
{
	class XMLElement;
}

typedef RSceneComponent* (*ComponentFactoryCreatePtr)(RSceneObject*);

/// Mapping class ids to their factory create functions
extern std::map<std::string, ComponentFactoryCreatePtr> ClassIdToFactoryCreate;

/// Helper class for statically registering scene component classes with their factory create functions
template<typename T>
class RSceneComponentStaticRegistrator
{
public:
	RSceneComponentStaticRegistrator()
	{
		const std::string ClassName = T::_StaticGetClassName();
		assert(ClassIdToFactoryCreate.find(ClassName) == ClassIdToFactoryCreate.end());
		ClassIdToFactoryCreate[ClassName] = &T::FactoryCreate;
	}
};

#define DECLARE_SCENE_COMPONENT(type, base) \
		typedef base Base; \
		friend class Base; \
		DECLARE_RUNTIME_TYPE(type, base) \
	public:\
		static std::unique_ptr<type> _CreateComponentUnique(RSceneObject* InOwner) { return std::unique_ptr<type>(new type(InOwner)); } \
		static RSceneComponent* FactoryCreate(RSceneObject* InOwner); \
	private:


#define IMPLEMENT_SCENE_COMPONENT(type) \
	static RSceneComponentStaticRegistrator<type> _StaticRegistrator; \
	RSceneComponent* type::FactoryCreate(RSceneObject* InOwner) { return InOwner->AddComponent(_CreateComponentUnique(InOwner)); }


/// Base scene component class
class RSceneComponent : public RRuntimeTypeObject
{
	DECLARE_RUNTIME_TYPE(RSceneComponent, RRuntimeTypeObject);
public:
	RSceneComponent(RSceneObject* InOwner);
	virtual ~RSceneComponent() {}

	void NotifyComponentAdded();

	virtual void LoadComponentFromXmlElement(tinyxml2::XMLElement* ComponentElem);
	virtual void SaveComponentToXmlElement(tinyxml2::XMLElement* ComponentElem) const;

	/// Get the scene object which is owning this component
	RSceneObject* GetOwner() const;

	const RAabb& GetLocalAabb() const;

	virtual void Update(float DeltaTime) {}
protected:
	/// Callback when component is added to a scene object
	virtual void OnComponentAdded() {}

	RAabb LocalBounds;

private:
	/// The scene object owning this component
	RSceneObject*	OwnerSceneObject;
};

FORCEINLINE RSceneObject* RSceneComponent::GetOwner() const
{
	return OwnerSceneObject;
}
