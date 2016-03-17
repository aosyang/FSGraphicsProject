//=============================================================================
// d3dUtil.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Commonly used D3D macros
//=============================================================================
#ifndef _D3DUTIL_H
#define _D3DUTIL_H

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

#define SAFE_RELEASE(p)       { if (p) { (p)->Release();  (p) = nullptr; } }
#define SAFE_ADDREF(p)        { if (p) { (p)->AddRef(); } }

#define SAFE_DELETE_ARRAY(p)  { delete [](p); p = nullptr; }
#define SAFE_DELETE(p)        { delete (p); p = nullptr;  }

#endif