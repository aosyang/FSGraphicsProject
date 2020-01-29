//=============================================================================
// RFileUtil.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

// Result for comparing timestamps of two files
enum class ETimestampComparison : UINT8
{
	InvalidFile,				// One or both files are invalid
	EarlierFirst,				// The first file is earlier than the second
	EarlierSecond,				// The second file is earlier than the first
	Equal,						// Both files have the same timestamp
};

class RFileUtil
{
public:
	/// Get file name part of a path
	static std::string GetFileNameInPath(const std::string& path);

	/// Replace extension in filename
	static std::string ReplaceExtension(const std::string& filename, const std::string& ext);

	/// Remove extension from filename ('File.ext' becomes 'File')
	static std::string StripExtension(const std::string& filename);

	/// Check if a path is relative
	static bool CheckIsRelativePath(const std::string& path);

	/// Check if a path exists
	static bool CheckPathExists(const std::string& Path);

	/// Create a directory
	static bool CreateDirectory(const std::string& PathName);

	/// Compare timestamps of two files
	static ETimestampComparison CompareFileTimestamp(const std::string& First, const std::string& Second);

	/// Push a new path as working path and store current one
	static void PushWorkingPath(const std::string& NewPath);

	/// Pop a stored path and make it current
	static void PopWorkingPath();

	/// Get full path of a relative path
	static std::string GetFullPath(const std::string& Path);

	static std::string CombinePath(const std::string& First, const std::string& Second);

	static std::string TrimLeadingSeperators(const std::string& Path);

	static std::string TrimTrailingSeperators(const std::string& Path);

	/// Invalid path string
	static const std::string InvalidPath;

private:
	static std::vector<std::string> WorkingPathStack;
};
