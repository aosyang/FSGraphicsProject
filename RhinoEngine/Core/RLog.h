//=============================================================================
// RLog.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RSingleton.h"

/// A helper class that outputs logs to multiple targets
class RLogOutputTargets : public RSingleton<RLogOutputTargets>
{
	friend class RSingleton<RLogOutputTargets>;
public:
	void Print(const char* Format, ...);

private:
	RLogOutputTargets();

	void OutputInternal(const char* Buffer);
};

#define GLogOutputTargets RLogOutputTargets::Instance()


/// Log with variable number of arguments
#define RLog(...)				{ GLogOutputTargets.Print(__VA_ARGS__); }

/// Log with warning prefix
#define RLogWarning(...)		{ GLogOutputTargets.Print("[Warning] "); RLog(__VA_ARGS__); }

/// Log with error prefix
#define RLogError(...)			{ GLogOutputTargets.Print("***Error*** "); RLog(__VA_ARGS__); }
