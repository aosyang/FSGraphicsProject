//=============================================================================
// StdHelper.h by Shiyang Ao, 2019 All Rights Reserved.
//
// Std container class helper functions
//=============================================================================

#pragma once

/// Check whether a given value exists in a container.
/// Note: Requires operator '==' to be overloaded for the value type.
template<typename T, typename V>
FORCEINLINE bool StdContains(const T& Container, const V& Value)
{
	return std::find(Container.begin(), Container.end(), Value) != Container.end();
}

/// Removes a value from a container.
/// Returns true if value is removed, false if value is not found in container.
template<typename T, typename V>
FORCEINLINE bool StdRemove(T& Container, const V& Value)
{
	auto Iter = std::find(Container.begin(), Container.end(), Value);
	if (Iter != Container.end())
	{
		Container.erase(Iter);
		return true;
	}

	return false;
}

/// Removes a value from a container.
/// An assert arises if value is not found in container.
template<typename T, typename V>
FORCEINLINE void StdRemoveChecked(T& Container, const V& Value)
{
	auto Iter = std::find(Container.begin(), Container.end(), Value);
	assert(Iter != Container.end());

	Container.erase(Iter);
}
