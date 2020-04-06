//=============================================================================
// RTimer.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RTimer.h"

#include <Windows.h>	// QueryPerformanceFrequency, QueryPerformanceCounter

RTimer::RTimer()
	: m_SecondsPerCount(0.0), m_DeltaTime(-1.0), m_BaseTime(0),
	  m_PausedTime(0), m_PrevTime(0), m_CurrTime(0), m_Stopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0 / (double)countsPerSec;
}

float RTimer::TotalTime() const
{
	if (m_Stopped)
	{
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}
	else
	{
		return (float)(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}
}

float RTimer::DeltaTime() const
{
	return (float)m_DeltaTime;
}

float RTimer::GetFramerate() const
{
	return (float)(1.0 / m_DeltaTime);
}

void RTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_Stopped = false;
}

void RTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// If we are resuming the timer from a stopped state...
	if (m_Stopped)
	{
		// then accumulate the paused time.
		m_PausedTime += (startTime - m_StopTime);

		// since we are starting the timer back up, the current
		// previous time is not valid, as it occurred while paused.
		// So reset it to the current time.
		m_PrevTime = startTime;

		// no longer stopped...
		m_StopTime = 0;
		m_Stopped = false;
	}
}

void RTimer::Stop()
{
	// If we are already stopped, then don't do anything.
	if (!m_Stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		// Otherwise, save the time we stopped at, and set
		// the Boolean flag indicating the timer is stopped.
		m_StopTime = currTime;
		m_Stopped = true;
	}
}

void RTimer::Tick()
{
	if (m_Stopped)
	{
		m_DeltaTime = 0.0;
		return;
	}

	// Get the time this frame.
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurrTime = currTime;

	// Time difference between this frame and the previous.
	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;

	// Prepare for next frame.
	m_PrevTime = m_CurrTime;

	// Force nonnegative.
	if (m_DeltaTime < 0.0)
	{
		m_DeltaTime = 0.0f;
	}
}
