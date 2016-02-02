//=============================================================================
// RSingleton.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Singleton template
//=============================================================================
#ifndef _RSINGLETON_H
#define _RSINGLETON_H

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

#endif
