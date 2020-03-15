//=============================================================================
// RSceneComponentFactory.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class RSceneObject;
class RSceneComponent;

/// Register all component classes defined in the engine
void RegisterEngineComponentClasses();

/// Create a component by registered class name
RSceneComponent* FactoryCreateSceneComponent(const std::string& ClassName, RSceneObject* InOwner);
