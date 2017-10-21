//=============================================================================
// RResourceBase.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Render resource interface class
//=============================================================================
#pragma once

#include "Core/REngine.h"

enum ResourceState
{
	RS_Empty,
	RS_EnqueuedForLoading,
	RS_Loaded,
};

enum ResourceType
{
	RT_Mesh,
	RT_Texture,
};

class RResourceBase
{
public:
	RResourceBase(ResourceType type, string path);
	virtual ~RResourceBase() = 0 {}

	ResourceState GetResourceState() const	{ return m_State; }
	ResourceType GetResourceType() const	{ return m_Type; }
	const string& GetPath() const			{ return m_ResourcePath; }

	/// Check if resource has been fully loaded
	bool IsLoaded() const					{ return m_State == RS_Loaded; }

	/// Callback when resource has been enqueued for loading
	virtual void OnEnqueuedForLoading();

	/// Callback when resource loading is complete
	virtual void OnLoadingFinished();

	/// Get the time when resource has been fully loaded
	float GetResourceTimestamp()			{ return m_LoadingFinishTime; }

private:
	ResourceState		m_State;
	ResourceType		m_Type;
	string				m_ResourcePath;
	float				m_LoadingFinishTime;
};
