//=============================================================================
// RResourceBase.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// Render resource interface class implementation
//=============================================================================
#include "Rhino.h"

#include "RResourceBase.h"


RResourceBase::RResourceBase(ResourceType type, string path)
	: m_State				(RS_Empty),
	  m_Type				(type),
	  m_ResourcePath		(path),
	  m_LoadingFinishTime	(0.0f)
{
}

void RResourceBase::OnEnqueuedForLoading()
{
	assert(m_State == RS_Empty);

	m_State = RS_EnqueuedForLoading;
}

void RResourceBase::OnLoadingFinished()
{
	assert(m_State != RS_Loaded);

	m_LoadingFinishTime = REngine::GetTimer().TotalTime();
	m_State = RS_Loaded;
}
