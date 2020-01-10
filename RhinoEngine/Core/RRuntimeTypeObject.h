//=============================================================================
// RRuntimeTypeObject.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "CoreTypes.h"
#include "RLog.h"

/// Runtime type info for scene object classes
struct RSceneObjectRuntimeTypeInfo
{
	RSceneObjectRuntimeTypeInfo(const char* ClassName)
	{
		TypeId = std::hash<std::string>{}(std::string(ClassName));
		RLog("Class \'%s\' has type id %zu\n", ClassName, TypeId);
	}

	/// Type id from hashed class name string
	size_t TypeId;
};

#define DECLARE_RUNTIME_TYPE(type)\
	public:\
		static size_t _StaticGetRuntimeTypeId() { static RSceneObjectRuntimeTypeInfo _RuntimeTypeInfo(#type); return _RuntimeTypeInfo.TypeId; }\
		virtual size_t GetRuntimeTypeId() const override { return type::_StaticGetRuntimeTypeId(); }\
	private:


class RRuntimeTypeObject
{
public:
	/// Returns runtime type id for the class
	virtual size_t GetRuntimeTypeId() const { return 0; }

	/// Dynamic-cast to another scene object type. Returns nullptr if types don't match
	/// TODO: Need inheritance support
	template<typename T>
	T* CastTo() { return IsType<T>() ? static_cast<T*>(this) : nullptr; }

	template<typename T>
	const T* CastTo() const { return IsType<T>() ? static_cast<const T*>(this) : nullptr; }

	/// Check if object matches type of given class
	template<typename T>
	bool IsType() const
	{
		return GetRuntimeTypeId() == T::_StaticGetRuntimeTypeId();
	}
};
