//=============================================================================
// D3DCommonPrivate.h by Shiyang Ao, 2019 All Rights Reserved.
//
// Includes common header files used by DirectX
//=============================================================================

#pragma once

#include <d3d11.h>

// ComPtr
#include <wrl.h>
using Microsoft::WRL::ComPtr;

// Definition of _com_error
#include <comdef.h>

#if defined(DEBUG) || defined(_DEBUG)
	#ifndef HR
	#define HR(x)											\
	{														\
		HRESULT hr = (x);									\
		if (!SUCCEEDED(hr)) {								\
			_com_error err(hr);								\
			LPCTSTR errMsg = err.ErrorMessage();			\
			OutputDebugString(errMsg);						\
			OutputDebugString(L"\n");						\
			MessageBox(0, errMsg, 0, MB_ICONERROR);			\
		}													\
		assert(SUCCEEDED(hr));								\
	}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif
