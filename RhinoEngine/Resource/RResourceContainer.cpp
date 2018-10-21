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
	UniqueLockWrapperImpl(MutexWrapper* Mutex)
		: UniqueLock(static_cast<MutexWrapperImpl*>(Mutex)->Mutex)
	{
	}

	unique_lock<mutex> UniqueLock;
};

MutexWrapper* MutexWrapper::Create()
{
	return new MutexWrapperImpl();
}

unique_ptr<UniqueLockWrapper> UniqueLockWrapper::Create(MutexWrapper* Mutex)
{
	return move(unique_ptr<UniqueLockWrapper>(new UniqueLockWrapperImpl(Mutex)));
}

RResourceContainerBase::RResourceContainerBase()
{
	ResourceMutex = MutexWrapper::Create();
}

RResourceContainerBase::~RResourceContainerBase()
{
	SAFE_DELETE(ResourceMutex);
}

void RResourceContainerBase::Lock()
{
	ResourceMutex->Lock();
}

void RResourceContainerBase::Unlock()
{
	ResourceMutex->Unlock();
}
