//=============================================================================
// RSceneComponentFactory.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RSceneComponentFactory.h"

#include "Core/StdHelper.h"
#include "RSceneComponent.h"

#include "RenderSystem/RDirectionalLightComponent.h"
#include "RenderSystem/RPointLightComponent.h"

namespace
{
	/// Factory function pointer for component
	typedef RSceneComponent* (*ComponentFactoryCreatePtr)(RSceneObject*);

	/// Mapping class ids to their factory create functions
	std::map<std::string, ComponentFactoryCreatePtr> ComponentClassToFactory;
}

#define RegisterComponentClass(type) \
	assert(ComponentClassToFactory.find(#type) == ComponentClassToFactory.end()); \
	ComponentClassToFactory[#type] = &type::FactoryCreate;

void RegisterEngineComponentClasses()
{
	// Add factory functions of components here
	RegisterComponentClass(RDirectionalLightComponent);
	RegisterComponentClass(RPointLightComponent);
}

RSceneComponent* FactoryCreateSceneComponent(const std::string& ClassName, RSceneObject* InOwner)
{
	if (ComponentClassToFactory.find(ClassName) != ComponentClassToFactory.end())
	{
		return (*ComponentClassToFactory[ClassName])(InOwner);
	}

	return nullptr;
}
