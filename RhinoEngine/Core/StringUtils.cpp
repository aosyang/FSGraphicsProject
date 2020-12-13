//=============================================================================
// StringUtils.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "StringUtils.h"

namespace StringUtils
{
	bool EqualsIgnoreCase(const std::string& a, const std::string& b)
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

	bool ToBool(const std::string& InString)
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
				return EqualsIgnoreCase(InString, StrTrue);
			}
		}

		return false;
	}


	std::vector<std::string> Split(const std::string& Str, const std::string& Delimiter)
	{
		std::vector<std::string> Outputs;
		size_t Start = 0;
		size_t End = Str.find(Delimiter);
		while (End != std::string::npos)
		{
			Outputs.push_back(Str.substr(Start, End - Start));
			Start = End + Delimiter.length();
			End = Str.find(Delimiter, Start);
		}
		Outputs.push_back(Str.substr(Start));

		return Outputs;
	}
}
