//=============================================================================
// RResourceContainer.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

// Case-insensitive string comparison
// Returns: 0 if both strings are equal;
//			-1 if string 1 is smaller;
//			1 if string 2 is smaller.
int strcasecmp(const char* str1, const char* str2);


// Note: Since std mutex header cannot be included in any public headers used by clr,
//		 wrapper classes are used to hide mutex declarations.

/// A wrapper class for std::mutex to avoid including mutex in engine public header files
class MutexWrapper
{
public:
	virtual ~MutexWrapper() {}

	virtual void Lock()		{}
	virtual void Unlock()	{}

	static std::unique_ptr<MutexWrapper> Create();
};

/// A wrapper class for std::unique_lock to avoid including mutex in engine public header files
class UniqueLockWrapper
{
public:
	virtual ~UniqueLockWrapper() {}

	static std::unique_ptr<UniqueLockWrapper> Create(std::unique_ptr<MutexWrapper>& Mutex);
};

/// Resource container interface
class RResourceContainerBase
{
public:
	RResourceContainerBase();
	virtual ~RResourceContainerBase() {}

	virtual void ReleaseAllResources() {}

protected:
	void Lock();
	void Unlock();

protected:
	std::unique_ptr<MutexWrapper>	ResourceMutex;
};

template<typename T>
class RResourceContainer : public RResourceContainerBase
{
private:
	std::vector<T*>	Resources;
public:

	/// Release all resources in the container
	virtual void ReleaseAllResources() override
	{
		Lock();

		for (auto Resource : Resources)
		{
			delete Resource;
		}
		Resources.clear();

		Unlock();
	}

	/// Add resource to container
	void Add(T* Resource)
	{
		Lock();
		Resources.push_back(Resource);
		Unlock();
	}

	/// Remove resource from container
	void Remove(T* Resource)
	{
		Lock();
		Resources.remove(Resource);
		Unlock();
	}

	/// Find resource by path
	T* Find(const std::string& Path)
	{
		return Find(Path.c_str());
	}

	/// Find resource by path
	T* Find(const char* Path)
	{
		std::unique_ptr<UniqueLockWrapper> UniqueLock = UniqueLockWrapper::Create(ResourceMutex);

		if (Path)
		{
			for (auto Iter : Resources)
			{
				if (strcasecmp(Iter->GetAssetPath().c_str(), Path) == 0)
				{
					return Iter;
				}

				// If searching path contains file name only, also try matching file names
				const std::string ResourceName = RFileUtil::GetFileNameInPath(Iter->GetAssetPath());
				if (strcasecmp(ResourceName.c_str(), Path) == 0)
				{
					return Iter;
				}
			}
		}

		return nullptr;
	}

	/// Get a copy of resource array
	std::vector<T*> GetResourceArrayCopy()
	{
		std::vector<T*> ArrayCopy;

		Lock();
		ArrayCopy = Resources;
		Unlock();

		return ArrayCopy;
	}
};

