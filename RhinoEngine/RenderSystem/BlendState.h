//=============================================================================
// BlendState.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

// Define blend state enumerations
#define DECLARE_BLEND_STATE(x) x,
enum class BlendState {
#include "BlendStateEnums.h"
	Count
};
#undef DECLARE_BLEND_STATE

// Declare blend state strings
extern const char* BlendStateNames[(int)BlendState::Count];

// Get a blend state enum by a name
bool BlendStateNameToEnum(const std::string& Name, BlendState& OutBlendState);
