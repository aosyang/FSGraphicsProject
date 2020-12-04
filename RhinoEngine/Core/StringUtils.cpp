//=============================================================================
// StringUtils.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "StringUtils.h"

namespace
{
	// Case insensitive comparison
	bool iequals(const std::string& a, const std::string& b)
	{
		if (a.size() != b.size())
		{
			return false;
		}

		for (size_t i = 0; i < a.size(); i++)
		{
			if (tolower(a[i]) != tolower(b[i]))
			{
				return false;
			}
		}

		return true;
	}
}

bool StringUtils::ToBool(const std::string& InString)
{
	if (InString.length() > 0)
	{
		if (isdigit(InString[0]))
		{
			if (std::stoi(InString) > 0)
			{
				return true;
			}
		}
		else if (isalpha(InString[0]))
		{
			static const std::string StrTrue("true");
			return iequals(InString, StrTrue);
		}
	}

	return false;
}
