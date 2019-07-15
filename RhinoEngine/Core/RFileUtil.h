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

	/// Remove extension from filename
	static string StripExtension(const string& filename);

	/// Check if a path is relative
	static bool CheckIsRelativePath(const string& path);

	/// Push a new path as working path and store current one
	static void PushWorkingPath(const string& NewPath);

	/// Pop a stored path and make it current
	static void PopWorkingPath();

	/// Get full path of a relative path
	static string GetFullPath(const string& Path);

	/// Invalid path string
	static const string InvalidPath;

private:
	static vector<string> WorkingPathStack;
};
