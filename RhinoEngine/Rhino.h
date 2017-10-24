//=============================================================================
// Rhino.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Rhino Engine main header file
//=============================================================================
#ifndef _RHINO_H
#define _RHINO_H

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <cmath>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include <sstream>
#include <iterator>
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

#include "../lua5.3/lua.hpp"

#include "Core/RVector.h"
#include "Core/RQuat.h"
#include "Core/RMatrix.h"
#include "Core/RTransform.h"

#include "Core/RColor.h"
#include "Core/RAabb.h"
#include "Core/RRay.h"

class RMesh;
class RTexture;
class RResourceBase;

// Engine Classes
#include "Collision/RCollision.h"

#include "Core/RSerializer.h"
#include "Core/RTimer.h"
#include "Core/RInput.h"
#include "Core/RLog.h"
#include "Core/IApp.h"
#include "Core/MathHelper.h"
#include "Core/RScriptSystem.h"
#include "Core/RFileUtil.h"

#include "../Shaders/ConstBufferVS.h"
#include "../Shaders/ConstBufferPS.h"

#include "RenderSystem/RVertexDeclaration.h"
#include "RenderSystem/RRenderSystem.h"
#include "RenderSystem/RMeshElement.h"
#include "RenderSystem/RDebugRenderer.h"
#include "RenderSystem/RText.h"
#include "RenderSystem/DDSTextureLoader.h"
#include "RenderSystem/RAnimation.h"
#include "RenderSystem/RResourceManager.h"
#include "RenderSystem/RShaderManager.h"
#include "RenderSystem/RShaderConstantBuffer.h"
#include "RenderSystem/RMesh.h"
#include "RenderSystem/RTexture.h"
#include "RenderSystem/RShadowMap.h"
#include "RenderSystem/RRenderMeshComponent.h"

#include "Core/RDebugMenu.h"
#include "Core/REngine.h"

#include "Scene/RSceneObject.h"
#include "Scene/RCamera.h"
#include "Scene/RSMeshObject.h"
#include "Scene/RSkybox.h"
#include "Scene/RScene.h"

#endif
