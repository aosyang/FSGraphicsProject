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
	RSceneObjectRuntimeTypeInfo(const char* ClassName, size_t InParentTypeId);

	/// Type id from hashed class name string
	size_t TypeId;
};

/// Declare functions for a runtime-type object
#define DECLARE_RUNTIME_TYPE(type, base)\
	public:\
		static RSceneObjectRuntimeTypeInfo& _StaticGetRuntimeTypeInfo()\
		{\
			static RSceneObjectRuntimeTypeInfo _RuntimeTypeInfo(#type, base::_StaticGetRuntimeTypeId());\
			return _RuntimeTypeInfo;\
		}\
		static size_t _StaticGetRuntimeTypeId()				{ return _StaticGetRuntimeTypeInfo().TypeId; }\
		virtual size_t GetRuntimeTypeId() const override	{ return type::_StaticGetRuntimeTypeId(); }\
	private:


/// Base class of any objects that require type query at runtime
class RRuntimeTypeObject
{
public:
	/// Returns runtime type id for the class
	virtual size_t GetRuntimeTypeId() const { return 0; }

	/// The runtime type id for base class
	static size_t _StaticGetRuntimeTypeId() { return 0; }

	/// Dynamic-cast to another runtime type. Returns null if types don't match
	template<typename T>
	T* CastTo() { return CanCastTo<T>() ? static_cast<T*>(this) : nullptr; }

	template<typename T>
	const T* CastTo() const { return CanCastTo<T>() ? static_cast<const T*>(this) : nullptr; }

	/// Check if object matches type of given class
	template<typename T>
	bool IsExactType() const
	{
		return GetRuntimeTypeId() == T::_StaticGetRuntimeTypeId();
	}

	/// Check if object is type or child type of given class
	template<typename T>
	bool CanCastTo() const
	{
		return IsTypeOrChildTypeOf(T::_StaticGetRuntimeTypeId());
	}

private:
	/// Check if object is type or child type of given type id
	bool IsTypeOrChildTypeOf(size_t OtherTypeId) const;
};
