//=============================================================================
// Rhino.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Rhino Engine main header file
//=============================================================================
#ifndef _RHINO_H
#define _RHINO_H

#include <Windows.h>

#include <cmath>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <vector>

// DirectX 3D
#include <d3d11.h>
#include <DirectXMath.h>
#include "RenderSystem/d3dUtil.h"

using namespace DirectX;


// Engine Classes
#include "Core/RTimer.h"
#include "Core/RInput.h"
#include "Core/IApp.h"
#include "Core/MathHelper.h"

#include "RenderSystem/RRenderSystem.h"
#include "RenderSystem/RMeshElement.h"
#include "RenderSystem/DDSTextureLoader.h"

#include "Core/REngine.h"

#endif
