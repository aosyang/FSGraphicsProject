//=============================================================================
// RSingleton.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Singleton template
//=============================================================================
#pragma once

template <typename T>
class RSingleton
{
protected:
	RSingleton() {}

public:
	virtual ~RSingleton() {}

	static T& Instance()
	{
		static T instance;
		return instance;
	}
};

