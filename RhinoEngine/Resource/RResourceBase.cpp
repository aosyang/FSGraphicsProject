//=============================================================================
// RResourceBase.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// Render resource interface class implementation
//=============================================================================
#include "Rhino.h"

#include "RResourceBase.h"


RResourceBase::RResourceBase(const std::string& path)
	: m_State				(RS_Empty),
	  m_FileSystemPath		(path),
	  m_LoadingFinishTime	(0.0f)
{
}

bool RResourceBase::LoadResourceData(bool bIsAsyncLoading)
{
	MetaData = std::make_unique<RResourceMetaData>();
	std::string MetaFileName = m_FileSystemPath + ".meta";
	MetaData->LoadFromFile(MetaFileName);

	if (LoadResourceImpl(bIsAsyncLoading))
	{
		OnLoadingFinished(bIsAsyncLoading);

		return true;
	}

	return false;
}

bool RResourceBase::AreReferencedResourcesLoaded() const
{
	auto ReferencedResources = EnumerateReferencedResources();

	for (auto Resource : ReferencedResources)
	{
		if (!Resource->IsLoaded())
		{
			return false;
		}
	}

	return true;
}

void RResourceBase::OnEnqueuedForLoading()
{
	assert(m_State == RS_Empty);

	m_State = RS_EnqueuedForLoading;
}

void RResourceBase::OnLoadingFinished(bool bIsAsyncLoading)
{
	assert(m_State != RS_Loaded);

	m_LoadingFinishTime = GEngine.GetTimer().TotalTime();
	m_State = RS_Loaded;

	// Notify event listeners when async loading is complete
	if (bIsAsyncLoading)
	{
		RResourceManager::Instance().AddPendingNotifyResource(this);
	}
}

std::vector<RResourceBase*> RResourceBase::EnumerateReferencedResources() const
{
	const static std::vector<RResourceBase*> EmptyResources;
	return EmptyResources;
}

bool RResourceBase::LoadResourceImpl(bool bIsAsyncLoading)
{
	// Base class does nothing
	return false;
}
