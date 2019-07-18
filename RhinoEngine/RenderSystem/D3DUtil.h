//=============================================================================
// D3DUtil.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Commonly used D3D macros
//=============================================================================
#pragma once

#define SAFE_RELEASE(p)       { if (p) { (p)->Release();  (p) = nullptr; } }
#define SAFE_ADDREF(p)        { if (p) { (p)->AddRef(); } }

#define SAFE_DELETE_ARRAY(p)  { delete [](p); p = nullptr; }
#define SAFE_DELETE(p)        { delete (p); p = nullptr;  }

