//=============================================================================
// RResourceContainer.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

// Note: Since std mutex header cannot be included in any public headers used by clr,
//		 wrapper classes are used to hide mutex declarations.

/// A wrapper class for std::mutex to avoid including mutex in engine public header files
class MutexWrapper
{
public:
	virtual ~MutexWrapper() {}

	virtual void Lock()		{}
	virtual void Unlock()	{}

	static unique_ptr<MutexWrapper> Create();
};

/// A wrapper class for std::unique_lock to avoid including mutex in engine public header files
class UniqueLockWrapper
{
public:
	virtual ~UniqueLockWrapper() {}

	static unique_ptr<UniqueLockWrapper> Create(unique_ptr<MutexWrapper>& Mutex);
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
	unique_ptr<MutexWrapper>	ResourceMutex;
};

template<typename T>
class RResourceContainer : public RResourceContainerBase
{
private:
	vector<T*>	Resources;
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
	T* Find(const string& Path)
	{
		return Find(Path.c_str());
	}

	/// Find resource by path
	T* Find(const char* Path)
	{
		unique_ptr<UniqueLockWrapper> UniqueLock = UniqueLockWrapper::Create(ResourceMutex);

		if (Path)
		{
			for (auto Iter : Resources)
			{
				if (strcasecmp(Iter->GetPath().c_str(), Path) == 0)
				{
					return Iter;
				}

				// If searching path contains file name only, also try matching file names
				const string ResourceName = RFileUtil::GetFileNameInPath(Iter->GetPath());
				if (strcasecmp(ResourceName.c_str(), Path) == 0)
				{
					return Iter;
				}
			}
		}

		return nullptr;
	}

	/// Get a copy of resource array
	vector<T*> GetResourceArrayCopy()
	{
		vector<T*> ArrayCopy;

		Lock();
		ArrayCopy = Resources;
		Unlock();

		return ArrayCopy;
	}

private:
	// case-insensitive string comparison
	static int strcasecmp(const char* str1, const char* str2)
	{
		int n = 0;
		while (str1[n] != 0 && str2[n] != 0)
		{
			char lower_ch1 = tolower(str1[n]);
			char lower_ch2 = tolower(str2[n]);
			if (lower_ch1 != lower_ch2)
				return lower_ch1 < lower_ch2 ? -1 : 1;
			n++;
		}

		if (str1[n] == 0 && str2[n] == 0)
			return 0;
		else if (str1[n] == 0)
			return -1;
		else
			return 1;
	}
};

