//=============================================================================
// Rhino.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Rhino Engine main header file
//=============================================================================
#pragma once

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

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

#include "Core/CoreTypes.h"

// DirectX 3D
#include <d3d11.h>
#include <d3dcompiler.h>
#include "RenderSystem/D3DUtil.h"
#pragma comment(lib, "d3dcompiler.lib")

#include "../lua5.3/lua.hpp"

#include "Core/RDelegate.h"
#include "Core/StdHelper.h"

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

#include "Resource/RResourceManager.h"

#include "RenderSystem/RVertexDeclaration.h"
#include "RenderSystem/RRenderSystem.h"
#include "RenderSystem/RDirectionalLightComponent.h"
#include "RenderSystem/RMeshElement.h"
#include "RenderSystem/RDebugRenderer.h"
#include "RenderSystem/RText.h"
#include "RenderSystem/DDSTextureLoader.h"
#include "RenderSystem/RAnimation.h"
#include "RenderSystem/RShaderManager.h"
#include "RenderSystem/RShaderConstantBuffer.h"
#include "RenderSystem/RMesh.h"
#include "RenderSystem/RTexture.h"
#include "RenderSystem/RShadowMap.h"
#include "RenderSystem/RRenderMeshComponent.h"
#include "RenderSystem/RPostProcessorManager.h"

#include "Physics/RPhysicsEngine.h"
#include "Physics/RRigidBodyComponent.h"

#include "Core/RDebugMenu.h"
#include "Core/REngine.h"

#include "Scene/RSceneManager.h"
#include "Scene/RSceneObject.h"
#include "Scene/RCamera.h"
#include "Scene/RSMeshObject.h"
#include "Scene/RSkybox.h"
#include "Scene/RScene.h"
