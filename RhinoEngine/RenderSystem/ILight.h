//=============================================================================
// ILight.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

enum class ELightType : UINT8
{
	DirectionalLight,
};

class ILight
{
public:
	virtual ~ILight() {}

	virtual ELightType GetLightType() = 0;
};