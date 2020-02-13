//=============================================================================
// RResourceBase.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Render resource interface class
//=============================================================================
#pragma once

#include "Core/REngine.h"
#include "RResourceMetaData.h"
#include "Core/RRuntimeTypeObject.h"

enum ResourceState
{
	RS_Empty,
	RS_EnqueuedForLoading,
	RS_Loaded,
};


/// Base resource class
class RResourceBase : public RRuntimeTypeObject
{
	DECLARE_RUNTIME_TYPE(RResourceBase, RRuntimeTypeObject)
public:
	RResourceBase(const std::string& path);
	virtual ~RResourceBase() = 0 {}

	/// Release any loaded content of this resource
	virtual void Reset() {};

	ResourceState GetResourceState() const	{ return m_State; }

	const RResourceMetaData& GetMetaData() const;
	RResourceMetaData& GetMetaData();

	/// Set the asset path of resource
	void SetAssetPath(const std::string& InAssetPath);

	/// Get the asset path of resource
	const std::string& GetAssetPath() const;

	/// Get a path of the resource used in the file system (eg. for serialization)
	const std::string& GetFileSystemPath() const;

	/// Check if resource has been fully loaded
	bool IsLoaded() const					{ return m_State == RS_Loaded; }

	bool LoadResourceData(bool bIsAsyncLoading);

	/// Release the resource then load it from the original asset again
	void Reload();

	/// Save any changes made to this asset
	bool SaveToDisk();

	/// Check if all referenced resources have been fully loaded
	bool AreReferencedResourcesLoaded() const;

	/// Callback when resource has been enqueued for loading
	virtual void OnEnqueuedForLoading();

	/// Callback when resource loading is complete
	virtual void OnLoadingFinished(bool bIsAsyncLoading);

	/// Get the time when resource has been fully loaded
	float GetResourceTimestamp()			{ return m_LoadingFinishTime; }

protected:
	/// Enumerate all resources been referenced directly by this resource
	virtual std::vector<RResourceBase*> EnumerateReferencedResources() const;

	virtual bool LoadResourceImpl();
	virtual bool SaveResourceImpl();

private:
	ResourceState		m_State;

	std::unique_ptr<RResourceMetaData> MetaData;

	/// The asset path used to access the resource in the engine
	std::string			m_AssetPath;

	/// Path to the resource file in file system
	std::string			m_FileSystemPath;
	float				m_LoadingFinishTime;
};

FORCEINLINE const RResourceMetaData& RResourceBase::GetMetaData() const
{
	return *MetaData;
}

FORCEINLINE RResourceMetaData& RResourceBase::GetMetaData()
{
	return *MetaData;
}

FORCEINLINE void RResourceBase::SetAssetPath(const std::string& InAssetPath)
{
	assert(InAssetPath.find("..") == std::string::npos);
	m_AssetPath = InAssetPath;
}

FORCEINLINE const std::string& RResourceBase::GetAssetPath() const
{
	return m_AssetPath;
}

FORCEINLINE const std::string& RResourceBase::GetFileSystemPath() const
{
	return m_FileSystemPath;
}
