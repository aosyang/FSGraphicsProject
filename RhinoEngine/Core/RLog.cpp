//=============================================================================
// RLog.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "RLog.h"

#include "Core/CoreTypes.h"


RLogOutputTargets::RLogOutputTargets()
{

}

void RLogOutputTargets::Print(const char* Format, ...)
{
	va_list Args;
	va_start(Args, Format);

	// Measure the size of output buffer
	int BufferSize = vsnprintf(nullptr, 0, Format, Args);

	char* Buffer = new char[BufferSize + 1];
	vsnprintf(Buffer, BufferSize + 1, Format, Args);

	va_end(Args);

	OutputInternal(Buffer);

	delete[] Buffer;
}

void RLogOutputTargets::OutputInternal(const char* Buffer)
{
	// Print to Visual Studio output window
	OutputDebugStringA(Buffer);

	// Print to console output
	std::cout << Buffer;
}
