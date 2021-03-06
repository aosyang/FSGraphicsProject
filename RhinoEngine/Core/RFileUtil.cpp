//=============================================================================
// RFileUtil.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "RFileUtil.h"

#include "RLog.h"

// Win32 file system APIs
#include <Shlwapi.h>

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

std::string RFileUtil::GetExtension(const std::string& filename, bool bHasDot /*= false*/)
{
	size_t dot_pos = filename.find_last_of('.');
	if (dot_pos != std::string::npos)
	{
		return bHasDot ? filename.substr(dot_pos) : filename.substr(dot_pos + 1);
	}

	return "";
}

std::string RFileUtil::GetExtensionInLowerCase(const std::string& filename, bool bHasDot /*= false*/)
{
	std::string FileExt = GetExtension(filename, bHasDot);
	for (UINT i = 0; i < FileExt.size(); i++)
	{
		FileExt[i] = tolower(FileExt[i]);
	}

	return FileExt;
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

std::vector<std::string> RFileUtil::GetFilesInDirectoryAndSubdirectories(const std::string& SearchPath, const std::string& FilePattern)
{
	std::vector<std::string> SearchResult;

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;

	// Load resources including sub-directories
	std::queue<std::string> dir_queue;
	dir_queue.push("");

	do
	{
		const std::string dir_name = dir_queue.front();
		dir_queue.pop();
		const std::string SearchingPath = SearchPath + dir_name + FilePattern;
		hFind = FindFirstFileA(SearchingPath.data(), &FindFileData);

		do
		{
			if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				const std::string FilePath = std::string("/") + dir_name + FindFileData.cFileName;
				SearchResult.push_back(FilePath);
			}
			else
			{
				if (FindFileData.cFileName[0] != '.')
				{
					dir_queue.push(dir_name + std::string(FindFileData.cFileName) + "/");
				}
			}

		} while (FindNextFileA(hFind, &FindFileData) != 0);
	} while (dir_queue.size());

	return SearchResult;
}

std::string RFileUtil::UnifyPathSeperators(const std::string& Path)
{
	std::string Result = Path;
	for (int i = 0; i < (int)Result.length(); i++)
	{
		if (Result[i] == '\\')
		{
			Result[i] = '/';
		}
	}

	// Remove redundant separators
	size_t pos = Result.find("//");
	while (pos != std::string::npos)
	{
		Result.replace(pos, 2, "/");
		pos = Result.find("//");
	}

	return Result;
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
