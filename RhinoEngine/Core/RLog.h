//=============================================================================
// RLog.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "CoreTypes.h"
#include "RSingleton.h"

// Enum for log verbosity level
enum class ELogVerbosity : UINT8
{
	Log,
	Debug,
	Verbose,
};

/// A helper class that outputs logs to multiple targets
class RLogOutputTargets : public RSingleton<RLogOutputTargets>
{
	friend class RSingleton<RLogOutputTargets>;
public:
	void Print(const char* Format, ...);

	void SetVerbosity(ELogVerbosity Verbosity);
	ELogVerbosity GetVerbosity() const;

private:
	RLogOutputTargets();

	// Output a string buffer
	void OutputInternal(const char* Buffer);

private:
	ELogVerbosity LogVerbosity;
};


FORCEINLINE void RLogOutputTargets::SetVerbosity(ELogVerbosity Verbosity)
{
	LogVerbosity = Verbosity;
}

FORCEINLINE ELogVerbosity RLogOutputTargets::GetVerbosity() const
{
	return LogVerbosity;
}

#define GLogOutputTargets RLogOutputTargets::Instance()


/// Log with variable number of arguments
#define RLog(...)							{ RLogWithVerbosity(ELogVerbosity::Log, __VA_ARGS__); }
#define RLogDebug(...)						{ RLogWithVerbosity(ELogVerbosity::Debug, __VA_ARGS__); }
#define RLogVerbose(...)					{ RLogWithVerbosity(ELogVerbosity::Verbose, __VA_ARGS__); }

/// Log with a verbosity level
#define RLogWithVerbosity(Verbosity, ...)	{ if (GLogOutputTargets.GetVerbosity() >= Verbosity) { GLogOutputTargets.Print(__VA_ARGS__); } }

/// Log with warning prefix
#define RLogWarning(...)					{ GLogOutputTargets.Print("[Warning] "); RLog(__VA_ARGS__); }

/// Log with error prefix
#define RLogError(...)						{ GLogOutputTargets.Print("***Error*** "); RLog(__VA_ARGS__); }
