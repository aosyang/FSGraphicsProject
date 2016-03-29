//=============================================================================
// Rhino.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Rhino Engine main header file
//=============================================================================
#ifndef _RHINO_H
#define _RHINO_H

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

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
#include <map>
#include <queue>
#include <string>
#include <algorithm>

using namespace std;

// DirectX 3D
#include <d3d11.h>
#include <d3dcompiler.h>
#include "RenderSystem/d3dUtil.h"
#pragma comment(lib, "d3dcompiler.lib")

#include "Core/RVector.h"
#include "Core/RMatrix.h"

#include "Core/RColor.h"
#include "Core/RAabb.h"
#include "Core/RRay.h"

class RMesh;
class RTexture;
class RBaseResource;

// Engine Classes
#include "Core/RTimer.h"
#include "Core/RInput.h"
#include "Core/IApp.h"
#include "Core/MathHelper.h"

#include "RenderSystem/RVertexDeclaration.h"
#include "RenderSystem/RRenderSystem.h"
#include "RenderSystem/RMeshElement.h"
#include "RenderSystem/DDSTextureLoader.h"
#include "RenderSystem/RResourceManager.h"
#include "RenderSystem/RShaderManager.h"
#include "RenderSystem/RMesh.h"
#include "RenderSystem/RTexture.h"

#include "Core/REngine.h"

#include "Scene/RSceneObject.h"
#include "Scene/RSMeshObject.h"
#include "Scene/RSkybox.h"

#endif
