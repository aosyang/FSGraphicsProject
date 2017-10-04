#include "Rhino.h"

#include "RMeshComponent.h"

RMeshComponent* RMeshComponent::Create(RSceneObject* InOwner)
{
	// TODO: Create component with render system
	return new RMeshComponent(InOwner);
}

RMeshComponent::RMeshComponent(RSceneObject* InOwner)
	: Base(InOwner)
{

}
