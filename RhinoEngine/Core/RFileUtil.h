//=============================================================================
// RFileUtil.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

namespace RFileUtil
{
	/// Replace extension in filename
	FORCEINLINE string ReplaceExt(const string& filename, const string& ext)
	{
		size_t pos = filename.find_last_of('.');

		if (pos != string::npos)
		{
			return filename.substr(0, pos + 1) + ext;
		}
		else if (filename.length() != 0)
		{
			return filename + ext;
		}

		return "";
	}
}
