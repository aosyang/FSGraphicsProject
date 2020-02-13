//=============================================================================
// BlendState.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "BlendState.h"

// Define blend state string
#define DECLARE_BLEND_STATE(x) #x,
const char* BlendStateNames[] = {
#include "BlendStateEnums.h"
};
#undef DECLARE_BLEND_STATE

bool BlendStateNameToEnum(const std::string& Name, BlendState& OutBlendState)
{
	for (int i = 0; i < ARRAYSIZE(BlendStateNames); i++)
	{
		if (strcmp(BlendStateNames[i], Name.c_str()) == 0)
		{
			OutBlendState = (BlendState)i;
			return true;
		}
	}

	return false;
}
