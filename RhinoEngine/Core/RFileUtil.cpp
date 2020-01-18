//=============================================================================
// RFileUtil.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

std::vector<std::string> RFileUtil::WorkingPathStack;
const std::string RFileUtil::InvalidPath("<InvalidPath>");

std::string RFileUtil::GetFileNameInPath(const std::string& path)
{
	size_t slash_pos = path.find_last_of("/\\");
	if (slash_pos == std::string::npos)
	{
		return path;
	}
	else
	{
		return path.substr(slash_pos + 1);
	}
}

std::string RFileUtil::ReplaceExtension(const std::string& filename, const std::string& ext)
{
	// Empty filename
	if (filename.length() == 0)
	{
		return "";
	}

	size_t slash_pos = filename.find_last_of("/\\");
	size_t pos = filename.find_last_of('.');

	// '.' is not found in filename
	if (pos == std::string::npos)
	{
		return filename + "." + ext;
	}

	// Make sure dot is after the last slash
	if ((slash_pos != std::string::npos && pos > slash_pos) || slash_pos == std::string::npos)
	{
		return filename.substr(0, pos + 1) + ext;
	}

	return filename + "." + ext;
}

std::string RFileUtil::StripExtension(const std::string& filename)
{
	// Empty filename
	if (filename.length() == 0)
	{
		return "";
	}
	
	size_t slash_pos = filename.find_last_of("/\\");
	size_t pos = filename.find_last_of('.');

	// No extension
	if (pos == std::string::npos)
	{
		return filename;
	}

	// Make sure dot is after the last slash
	if ((slash_pos != std::string::npos && pos > slash_pos) || slash_pos == std::string::npos)
	{
		return filename.substr(0, pos);
	}

	// Otherwise, dot comes before slash. Filename contains not extension
	return filename;
}

bool RFileUtil::CheckIsRelativePath(const std::string& path)
{
	return PathIsRelativeA(path.c_str()) == TRUE;
}

bool RFileUtil::CheckPathExists(const std::string& Path)
{
	return PathFileExistsA(Path.c_str()) == TRUE;
}

bool RFileUtil::CreateDirectory(const std::string& PathName)
{
	return CreateDirectoryA(PathName.c_str(), NULL) == TRUE;
}

ETimestampComparison RFileUtil::CompareFileTimestamp(const std::string& First, const std::string& Second)
{
	HANDLE hFirstFile = CreateFileA(First.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFirstFile == INVALID_HANDLE_VALUE)
	{
		return ETimestampComparison::InvalidFile;
	}

	HANDLE hSecondFile = CreateFileA(Second.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hSecondFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFirstFile);
		return ETimestampComparison::InvalidFile;
	}

	FILETIME FirstTimestamp, SecondTimestamp;
	GetFileTime(hFirstFile, NULL, NULL, &FirstTimestamp);
	GetFileTime(hSecondFile, NULL, NULL, &SecondTimestamp);

	CloseHandle(hFirstFile);
	CloseHandle(hSecondFile);

	LONG Result = CompareFileTime(&FirstTimestamp, &SecondTimestamp);

	if (Result == -1)
	{
		return ETimestampComparison::EarlierFirst;
	}
	else if (Result == 1)
	{
		return ETimestampComparison::EarlierSecond;
	}
	else
	{
		return ETimestampComparison::Equal;
	}
}

void RFileUtil::PushWorkingPath(const std::string& NewPath)
{
	char pWorkingPath[1024];
	GetCurrentDirectoryA(1024, pWorkingPath);

	WorkingPathStack.push_back(std::string(pWorkingPath));

	SetCurrentDirectoryA(NewPath.c_str());
	GetCurrentDirectoryA(1024, pWorkingPath);
	RLog("Working path has changed to: %s\n", pWorkingPath);
}

void RFileUtil::PopWorkingPath()
{
	assert(WorkingPathStack.size() > 0);

	int NumPaths = (int)WorkingPathStack.size();
	const std::string& PrevPath = WorkingPathStack[NumPaths - 1];
	SetCurrentDirectoryA(PrevPath.c_str());
	RLog("Working path has changed to: %s\n", PrevPath.c_str());

	WorkingPathStack.pop_back();
}

std::string RFileUtil::GetFullPath(const std::string& Path)
{
	char FullPath[MAX_PATH + 1];
	GetFullPathNameA(Path.c_str(), MAX_PATH, FullPath, nullptr);

	return std::string(FullPath);
}

std::string RFileUtil::CombinePath(const std::string& First, const std::string& Second)
{
	return RFileUtil::TrimTrailingSeperators(First) + "/" + RFileUtil::TrimLeadingSeperators(Second);
}

std::string RFileUtil::TrimLeadingSeperators(const std::string& Path)
{
	std::string TrimedPath(Path);
	const static std::string Seperators("\\/");

	// Trim leading slashes
	size_t Pos = TrimedPath.find_first_not_of(Seperators);
	if (Pos != std::string::npos)
	{
		TrimedPath = TrimedPath.substr(Pos);
	}

	return TrimedPath;
}

std::string RFileUtil::TrimTrailingSeperators(const std::string& Path)
{
	std::string TrimedPath(Path);
	const static std::string Seperators("\\/");

	// Trim trailing slashes
	size_t Pos = TrimedPath.find_last_not_of(Seperators);
	if (Pos != std::string::npos)
	{
		TrimedPath.resize(Pos + 1);
	}

	return TrimedPath;
}
