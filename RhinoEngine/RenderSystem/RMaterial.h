//=============================================================================
// RMaterial.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Resource/RResourceBase.h"

struct RShader;
class RTexture;
class RSerializer;

struct RMaterial
{
	RShader*					Shader;
	int							TextureNum;		/// Texture numbers, max 8
	RTexture*					Textures[8];

	/// Binary serialization operation
	void Serialize(RSerializer& serializer);

	/// Load an array of materials from xml file
	static bool LoadFromXmlFile(const std::string& Filename, std::vector<RMaterial>& OutMaterials);
};
