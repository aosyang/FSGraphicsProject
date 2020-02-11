//=============================================================================
// RMaterial.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RMaterial.h"


#include "tinyxml2/tinyxml2.h"

void RMeshMaterialData::Serialize(RSerializer& serializer)
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

std::vector<std::string> RMeshMaterialData::LoadFromXmlFile(const std::string& Filename)
{
	std::vector<std::string> MaterialList;

	std::unique_ptr<tinyxml2::XMLDocument> XmlDoc(new tinyxml2::XMLDocument());
	if (XmlDoc->LoadFile(Filename.c_str()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement* root = XmlDoc->RootElement();
		tinyxml2::XMLElement* elem = root->FirstChildElement("MeshElement");
		while (elem)
		{
			std::string MaterialName = "";
			const char* Text = elem->GetText();
			if (Text != nullptr)
			{
				MaterialName = Text;
			}

			MaterialList.push_back(MaterialName);
			elem = elem->NextSiblingElement();
		}
	}

	return MaterialList;
}

void RTextureSlotData::Serialize(RSerializer& Serializer)
{
	if (Serializer.IsReading())
	{
		std::string textureName;
		Serializer.SerializeData(textureName);
		Texture = RResourceManager::Instance().FindResource<RTexture>(textureName.c_str());
		if (!Texture)
		{
			Texture = RResourceManager::Instance().LoadResource<RTexture>(textureName.c_str(), EResourceLoadMode::Immediate);
		}

		Serializer.SerializeData(SlotId);
	}
	else
	{
		std::string textureName = Texture->GetAssetPath();
		Serializer.SerializeData(textureName);
		Serializer.SerializeData(SlotId);
	}
}

RMaterial::RMaterial(const std::string& Path)
	: RResourceBase(Path)
	, Shader(nullptr)
{

}

std::vector<std::string> RMaterial::GetSupportedExtensions()
{
	static const std::vector<std::string> MaterialExts{ ".material" };
	return MaterialExts;
}

void RMaterial::Serialize(RSerializer& serializer)
{
	if (serializer.IsReading())
	{
		std::string shaderName;
		serializer.SerializeData(shaderName);
		Shader = RShaderManager::Instance().GetShaderResource(shaderName.c_str());
		if (Shader == nullptr)
		{
			Shader = RShaderManager::Instance().GetShaderResource("Default");
		}
		serializer.SerializeVector(TextureSlots, &RSerializer::SerializeObject);
	}
	else
	{
		std::string shaderName;
		if (Shader)
		{
			shaderName = Shader->GetName();
		}

		if (shaderName == "")
		{
			shaderName = "Default";
		}

		serializer.SerializeData(shaderName);
		serializer.SerializeVector(TextureSlots, &RSerializer::SerializeObject);
	}
}

void RMaterial::SetTextureSlot(int Slot, RTexture* Texture)
{
	for (int i = 0; i < (int)TextureSlots.size(); i++)
	{
		if (TextureSlots[i].SlotId == Slot)
		{
			TextureSlots[i].Texture = Texture;
			return;
		}
	}

	TextureSlots.push_back(RTextureSlotData(Texture, Slot));
}

RMaterial* RMaterial::GetDefault()
{
	static RMaterial* DefaultMaterial = nullptr;
	if (DefaultMaterial == nullptr)
	{
		RShader* DefaultShader = RShaderManager::Instance().GetShaderResource("Default");

		// Make sure we're not getting the default material before the default shader is loaded
		assert(DefaultShader);

		DefaultMaterial = RResourceManager::Instance().CreateNewResource<RMaterial>("DefaultMaterial");
		DefaultMaterial->SetAssetPath("DefaultMaterial");
		DefaultMaterial->Shader = DefaultShader;
	}

	return DefaultMaterial;
}

bool RMaterial::LoadResourceImpl(bool bIsAsyncLoading)
{
	std::unique_ptr<tinyxml2::XMLDocument> XmlDoc = std::make_unique<tinyxml2::XMLDocument>();
	if (XmlDoc->LoadFile(GetFileSystemPath().c_str()) == tinyxml2::XML_SUCCESS)
	{
		std::vector<RMeshMaterialData> xmlMaterials;

		// <Material Shader="MyShaderName">
		tinyxml2::XMLElement* RootElem = XmlDoc->RootElement();

		const char* ShaderName = RootElem->Attribute("Shader");
		Shader = ShaderName ? RShaderManager::Instance().GetShaderResource(ShaderName) : nullptr;

		// <Texture Slot="0">Path/To/Texture</Texture>
		tinyxml2::XMLElement* TextureElem = RootElem->FirstChildElement("Texture");
		while (TextureElem)
		{
			int SlotId = -1;
			TextureElem->QueryIntAttribute("Slot", &SlotId);

			std::string textureName = TextureElem->GetText();
			RTexture* texture = RResourceManager::Instance().FindResource<RTexture>(textureName);

			// Load the texture immediately if haven't done so
			if (!texture)
			{
				texture = RResourceManager::Instance().LoadResource<RTexture>(textureName, EResourceLoadMode::Immediate);
			}

			TextureSlots.push_back(RTextureSlotData(texture, SlotId));
			TextureElem = TextureElem->NextSiblingElement();
		}

		return true;
	}

	return false;
}
