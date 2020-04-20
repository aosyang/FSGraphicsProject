//=============================================================================
// Platform.h by Shiyang Ao, 2020 All Rights Reserved.
//
// Header file for platform-specific definitions
//=============================================================================

#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define PLATFORM_WINDOWS 1
#elif __APPLE__
    #if TARGET_OS_MAC
        #define PLATFORM_MACOS 1
    #endif  // TARGET_OS_MAC
#endif

#ifndef PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 0
#endif

#ifndef PLATFORM_MACOS
#define PLATFORM_MACOS 0
#endif

// Platform headers and defines
#if PLATFORM_WINDOWS

#include <winerror.h>   // FORCEINLINE
#include <windows.h>

#else

#define FORCEINLINE __attribute__((always_inline))

void DebugBreak()
{
    __asm__("int $3");
}

#endif  // PLATFORM_WINDOWS
