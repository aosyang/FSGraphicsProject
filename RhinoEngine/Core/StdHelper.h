//=============================================================================
// StdHelper.h by Shiyang Ao, 2019 All Rights Reserved.
//
// Std container class helper functions
//=============================================================================

#pragma once

/// Check whether a given value exists in the container.
/// Note: Requires operator '==' to be overloaded for the value type.
template<typename T>
FORCEINLINE bool StdContains(const std::vector<T>& Container, const T& Value)
{
	return std::find(Container.begin(), Container.end(), Value) != Container.end();
}
