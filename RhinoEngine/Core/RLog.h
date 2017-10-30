//=============================================================================
// RLog.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include <stdio.h>

#define LogBufSize 1024

/// Log with variable number of arguments
#define RLog(...)				{ char LogMsg[LogBufSize]; sprintf_s(LogMsg, __VA_ARGS__); OutputDebugStringA(LogMsg); }

/// Log with warning prefix
#define RLogWarning(...)		{ OutputDebugStringA("[Warning] "); RLog(__VA_ARGS__); }

/// Log with error prefix
#define RLogError(...)			{ OutputDebugStringA("***Error*** "); RLog(__VA_ARGS__); }
