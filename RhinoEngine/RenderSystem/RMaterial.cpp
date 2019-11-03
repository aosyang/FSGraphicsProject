//=============================================================================
// RMaterial.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RMaterial.h"


#include "tinyxml2/tinyxml2.h"

void RMaterial::Serialize(RSerializer& serializer)
{
	if (serializer.IsReading())
	{
		std::string shaderName;
		serializer.SerializeData(shaderName);
		Shader = RShaderManager::Instance().GetShaderResource(shaderName.c_str());

		serializer.SerializeData(TextureNum);

		int i;
		for (i = 0; i < TextureNum; i++)
		{
			std::string textureName;
			serializer.SerializeData(textureName);
			Textures[i] = RResourceManager::Instance().FindResource<RTexture>(textureName.c_str());
			if (!Textures[i])
				Textures[i] = RResourceManager::Instance().LoadResource<RTexture>(textureName.c_str(), EResourceLoadMode::Immediate);
		}

		for (; i < 8; i++)
		{
			Textures[i] = nullptr;
		}
	}
	else
	{
		std::string shaderName;
		if (Shader)
			shaderName = Shader->GetName();

		serializer.SerializeData(shaderName);
		serializer.SerializeData(TextureNum);

		int i;
		for (i = 0; i < TextureNum; i++)
		{
			std::string textureName = Textures[i]->GetAssetPath();
			serializer.SerializeData(textureName);
		}
	}
}

bool RMaterial::LoadFromXmlFile(const std::string& Filename, std::vector<RMaterial>& OutMaterials)
{
	std::unique_ptr<tinyxml2::XMLDocument> XmlDoc(new tinyxml2::XMLDocument());
	if (XmlDoc->LoadFile(Filename.c_str()) == tinyxml2::XML_SUCCESS)
	{
		std::vector<RMaterial> xmlMaterials;

		tinyxml2::XMLElement* root = XmlDoc->RootElement();
		tinyxml2::XMLElement* elem = root->FirstChildElement("MeshElement");
		while (elem)
		{
			const char* shaderName = elem->Attribute("Shader");
			RMaterial material = { nullptr, 0 };
			material.Shader = RShaderManager::Instance().GetShaderResource(shaderName);

			tinyxml2::XMLElement* elem_tex = elem->FirstChildElement();
			while (elem_tex)
			{
				std::string textureName = elem_tex->GetText();
				RTexture* texture = RResourceManager::Instance().FindResource<RTexture>(textureName);

				if (!texture)
				{
					texture = RResourceManager::Instance().LoadResource<RTexture>(textureName, EResourceLoadMode::Immediate);
				}

				material.Textures[material.TextureNum++] = texture;
				elem_tex = elem_tex->NextSiblingElement();
			}

			xmlMaterials.push_back(material);
			elem = elem->NextSiblingElement();
		}

		if (xmlMaterials.size())
		{
			OutMaterials = xmlMaterials;
			return true;
		}
	}

	return false;
}
