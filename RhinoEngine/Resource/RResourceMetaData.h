//=============================================================================
// RResourceMetaData.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class RResourceMetaData
{
public:
	void LoadFromFile(const std::string& Filename);
	void SaveToFile(const std::string& Filename);

	void AddAttribute(const std::string& Key, const std::string& Value);
	void RemoveAttribute(const std::string& Key);

	const std::string& operator[](const std::string& Key) const;

private:
	std::map<std::string, std::string> Attributes;
};
