//=============================================================================
// RRenderSystemTypes.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

enum class ERenderPass : UINT8
{
	Background,		// Skybox pass
	SceneObject,	// Scene object pass
	Foreground,		// Objects that are not be blocked by any other objects

	NumPasses,
};

struct RenderViewInfo
{
	RenderViewInfo(RFrustum* InFrustum, ERenderPass InPass = ERenderPass::SceneObject)
		: Frustum(InFrustum)
		, RenderPass(InPass)
	{
	}

	RFrustum*	Frustum;
	ERenderPass RenderPass;
};
