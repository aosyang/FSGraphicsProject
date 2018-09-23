//=============================================================================
// TypeUtils.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

using namespace System;

namespace ManagedEngineWrapper
{
	public ref class TypeUtils
	{
	public:
		static String^ Float3ToString(float x, float y, float z)
		{
			return String::Format(L"{0}, {1}, {2}", x, y, z);
		}

		static void StringToFloat3(String^ str, float& x, float &y, float &z)
		{
			String^ delimStr = ",";
			cli::array<Char>^ delimiter = delimStr->ToCharArray();
			cli::array<String^>^ words = str->Split(delimiter);

			x = (float)Convert::ToDouble(words[0]);
			y = (float)Convert::ToDouble(words[1]);
			z = (float)Convert::ToDouble(words[2]);
		}
	};
}
