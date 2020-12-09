//=============================================================================
// AnimCommon.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

// 0 - Simple linear matrices blending.
//	   Fast but could be wrong in the cases where matrix rotations are significantly different.
// 1 - Decompose matrices and slerp rotations for better blending results
#define USE_MATRIX_DECOMPOSITION_IN_POSE_BLENDING 0

// 0 - Evaluate animation poses with local space bone matrices.
//	   The final bone pose is evaluated by combining bone transforms along the hierarchy.
// 1 - Evaluate animation poses with mesh space bone matrices.
//	   The bone matrix is the final bone pose.
#define USE_LOCAL_SPACE_BONE_POSE_EVALUTION 1
