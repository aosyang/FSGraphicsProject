//=============================================================================
// CoreTypes.h by Shiyang Ao, 2019 All Rights Reserved.
//
// Header file of common engine types
//=============================================================================

#pragma once

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

#include <memory>
#include <functional>

// STL libraries
#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <unordered_map>
#include <string>
#include <algorithm>

#include <Windows.h>

#include "Core/RVector.h"
#include "Core/RQuat.h"
#include "Core/RMatrix.h"
#include "Core/RTransform.h"

#include "Core/RColor.h"
#include "Core/RAabb.h"
#include "Core/RRay.h"


template<class T>
inline void HashCombine(size_t& HashValue, const T& Element)
{
	std::hash<T> Hash;
	HashValue ^= Hash(Element) + 0x9e3779b9 + (HashValue << 6) + (HashValue >> 2);
}

