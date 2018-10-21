//=============================================================================
// RResourceContainer.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RResourceContainer.h"

#include <mutex>

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
