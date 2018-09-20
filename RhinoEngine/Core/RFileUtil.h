//=============================================================================
// RFileUtil.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

class RFileUtil
{
public:
	/// Get file name part of a path
	static string GetFileNameInPath(const string& path);

	/// Replace extension in filename
	static string ReplaceExtension(const string& filename, const string& ext);

	/// Push a new path as working path and store current one
	static void PushWorkingPath(const char* NewPath);

	/// Pop a stored path and make it current
	static void PopWorkingPath();

private:
	static vector<string> WorkingPathStack;
};
