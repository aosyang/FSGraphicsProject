//=============================================================================
// RRenderSystemTypes.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

enum class ERenderPass : uint8_t
{
	Background,		// Skybox pass
	SceneObject,	// Scene object pass
	Foreground,		// Objects that are not be blocked by any other objects

	NumPasses,
};

enum class EPrimitiveTopology : uint8_t
{
	PointList,
	LineList,
	TriangleList,
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
