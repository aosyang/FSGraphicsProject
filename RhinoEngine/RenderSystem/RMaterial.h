//=============================================================================
// RMaterial.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

struct RShader;
class RTexture;
class RSerializer;

struct RMaterial
{
	RShader*					Shader;
	int							TextureNum;		/// Texture numbers, max 8
	RTexture*					Textures[8];

	void Serialize(RSerializer& serializer);

	/// Load an array of materials from xml file
	static bool LoadFromXmlFile(const string& Filename, vector<RMaterial>& OutMaterials);
};
