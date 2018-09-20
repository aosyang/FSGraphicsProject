//=============================================================================
// RFileUtil.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

vector<string> RFileUtil::WorkingPathStack;

string RFileUtil::GetFileNameInPath(const string& path)
{
	size_t slash_pos = path.find_last_of("/\\");
	if (slash_pos == string::npos)
	{
		return path;
	}
	else
	{
		return path.substr(slash_pos + 1);
	}
}

string RFileUtil::ReplaceExtension(const string& filename, const string& ext)
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

void RFileUtil::PushWorkingPath(const char* NewPath)
{
	char pWorkingPath[1024];
	GetCurrentDirectoryA(1024, pWorkingPath);

	WorkingPathStack.push_back(string(pWorkingPath));

	SetCurrentDirectoryA(NewPath);
	RLog("New working path is: %s\n", NewPath);
}

void RFileUtil::PopWorkingPath()
{
	assert(WorkingPathStack.size() > 0);

	int NumPaths = (int)WorkingPathStack.size();
	const string& PrevPath = WorkingPathStack[NumPaths - 1];
	SetCurrentDirectoryA(PrevPath.c_str());
	RLog("New working path is: %s\n", PrevPath.c_str());

	WorkingPathStack.pop_back();
}
