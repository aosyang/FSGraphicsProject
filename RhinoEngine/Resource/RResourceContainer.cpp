//=============================================================================
// RResourceContainer.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RResourceContainer.h"

#include <mutex>

int strcasecmp(const char* str1, const char* str2)
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


class MutexWrapperImpl : public MutexWrapper
{
public:
	virtual void Lock() override
	{
		Mutex.lock();
	}

	virtual void Unlock() override
	{
		Mutex.unlock();
	}

	mutex Mutex;
};

class UniqueLockWrapperImpl : public UniqueLockWrapper
{
public:
	UniqueLockWrapperImpl(unique_ptr<MutexWrapper>& Mutex)
		: UniqueLock(static_cast<MutexWrapperImpl*>(Mutex.get())->Mutex)
	{
	}

	unique_lock<mutex> UniqueLock;
};

unique_ptr<MutexWrapper> MutexWrapper::Create()
{
	return move(unique_ptr<MutexWrapper>(new MutexWrapperImpl()));
}

unique_ptr<UniqueLockWrapper> UniqueLockWrapper::Create(unique_ptr<MutexWrapper>& Mutex)
{
	return move(unique_ptr<UniqueLockWrapper>(new UniqueLockWrapperImpl(Mutex)));
}

RResourceContainerBase::RResourceContainerBase()
{
	ResourceMutex = MutexWrapper::Create();
}

void RResourceContainerBase::Lock()
{
	ResourceMutex->Lock();
}

void RResourceContainerBase::Unlock()
{
	ResourceMutex->Unlock();
}