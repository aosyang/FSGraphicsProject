//=============================================================================
// RRuntimeTypeObject.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RRuntimeTypeObject.h"

#include "CoreTypes.h"
#include "RLog.h"

/// A unordered map that holds pairs for all runtime types and their parent types
std::unordered_map<size_t, size_t> RuntimeTypeParents;

/// A map that converts runtime type ids to their names
std::map<size_t, std::string> RuntimeTypeIdToName;

RRuntimeTypeInfoData::RRuntimeTypeInfoData(const char* InClassName, size_t InParentTypeId)
	: TypeId(std::hash<std::string>{}(std::string(InClassName)))
	, ClassName(InClassName)
{
	// Check for hash collisions
	assert(RuntimeTypeIdToName.count(TypeId) == 0);

	RuntimeTypeIdToName[TypeId] = ClassName;

	RuntimeTypeParents.insert({ TypeId, InParentTypeId });
	RLogDebug("Class \'%s\' has type id %zu\n", ClassName, TypeId);
}

bool RRuntimeTypeObject::IsTypeOrChildTypeOf(size_t OtherTypeId) const
{
	const size_t ThisTypeId = GetRuntimeTypeId();
	if (ThisTypeId == OtherTypeId)
	{
		return true;
	}

	size_t ParentTypeId = RuntimeTypeParents[ThisTypeId];
	do
	{
		if (ParentTypeId == OtherTypeId)
		{
			return true;
		}

		ParentTypeId = RuntimeTypeParents[ParentTypeId];
	} while (ParentTypeId != 0);

	return false;
}
