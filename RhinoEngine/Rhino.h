//=============================================================================
// Rhino.h by Shiyang Ao, 2016 All Rights Reserved.
//
// Rhino Engine main header file
//=============================================================================
#pragma once

#include "Core/Platform.h"

#if PLATFORM_WINDOWS
// Win32 memory leak detection (works in debug builds only)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// Win32 file system APIs
#pragma comment(lib, "Shlwapi.lib")
#endif	// PLATFORM_WINDOWS

#include "Core/CoreTypes.h"

// DirectX 3D
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")			// For D3D11 shader reflection

#include "../lua5.3/lua.hpp"

#include "Core/RDelegate.h"
#include "Core/StdHelper.h"

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

#include "Resource/RResourceManager.h"

#include "RenderSystem/RVertexDeclaration.h"
#include "RenderSystem/RRenderSystem.h"
#include "RenderSystem/RDirectionalLightComponent.h"
#include "RenderSystem/RPointLightComponent.h"
#include "RenderSystem/RMeshElement.h"
#include "RenderSystem/RDebugRenderer.h"
#include "RenderSystem/RText.h"
#include "RenderSystem/RShaderManager.h"
#include "RenderSystem/RShaderConstantBuffer.h"
#include "RenderSystem/RMesh.h"
#include "RenderSystem/RTexture.h"
#include "RenderSystem/RShadowMap.h"
#include "RenderSystem/RRenderMeshComponent.h"
#include "RenderSystem/RPostProcessorManager.h"

#include "Animation/RAnimation.h"
#include "Animation/RAnimGraph.h"
#include "Animation/RAnimNode_Base.h"
#include "Animation/RAnimNode_AnimationPlayer.h"

#include "Physics/RPhysicsEngine.h"
#include "Physics/RRigidBodyComponent.h"

#include "AI/NavigationSystem/RNavigationSystem.h"
#include "AI/RAINavigationComponent.h"

#include "Core/RDebugMenu.h"
#include "Core/REngine.h"

#include "Scene/RSceneManager.h"
#include "Scene/RSceneObject.h"
#include "Scene/RCamera.h"
#include "Scene/RSMeshObject.h"
#include "Scene/RSkybox.h"
#include "Scene/RScene.h"

#include "imgui/imgui.h"
#include "UI/RProgressBar.h"
