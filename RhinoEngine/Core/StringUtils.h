//=============================================================================
// StringUtils.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "CoreTypes.h"

namespace StringUtils
{
	// Compare two strings but ignore their cases
	bool EqualsIgnoreCase(const std::string& a, const std::string& b);

	// Convert a string to bool
	bool ToBool(const std::string& InString);

	// Split a string by a delimiter and store the result in a vector
	std::vector<std::string> Split(const std::string& Str, const std::string& Delimiter);
}

