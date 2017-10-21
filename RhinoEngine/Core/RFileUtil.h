//=============================================================================
// RFileUtil.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

namespace RFileUtil
{
	/// Replace extension in filename
	FORCEINLINE string ReplaceExtension(const string& filename, const string& ext)
	{
		// Empty filename
		if (filename.length() == 0)
		{
			return "";
		}

		size_t slash_pos = filename.find_last_of("/\\");
		size_t pos = filename.find_last_of('.');

		// '.' is not found in filename
		if (pos == string::npos)
		{
			return filename + "." + ext;
		}

		if ((slash_pos != string::npos && pos > slash_pos) || slash_pos == string::npos)
		{
			return filename.substr(0, pos + 1) + ext;
		}

		return filename + "." + ext;
	}
}