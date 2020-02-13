//=============================================================================
// RResourceMetaData.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RResourceMetaData.h"
#include "tinyxml2/tinyxml2.h"
#include "Core/RFileUtil.h"

void RResourceMetaData::LoadFromFile(const std::string& Filename)
{
	std::unique_ptr<tinyxml2::XMLDocument> XmlDoc = std::make_unique<tinyxml2::XMLDocument>();

	if (XmlDoc->LoadFile(Filename.c_str()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement* MetaElem = XmlDoc->FirstChildElement("Metadata");
		if (MetaElem)
		{
			const tinyxml2::XMLAttribute* XmlAttribute = MetaElem->FirstAttribute();
			while (XmlAttribute)
			{
				Attributes[XmlAttribute->Name()] = XmlAttribute->Value();
				XmlAttribute = XmlAttribute->Next();
			}
		}
	}
}

void RResourceMetaData::SaveToFile(const std::string& Filename)
{
	if (Attributes.size() > 0)
	{
		std::unique_ptr<tinyxml2::XMLDocument> XmlDoc = std::make_unique<tinyxml2::XMLDocument>();
		XmlDoc->InsertEndChild(XmlDoc->NewDeclaration());
		tinyxml2::XMLElement* XmlElemMeta = XmlDoc->NewElement("Metadata");

		for (auto Iter : Attributes)
		{
			XmlElemMeta->SetAttribute(Iter.first.c_str(), Iter.second.c_str());
		}

		XmlDoc->InsertEndChild(XmlElemMeta);
		XmlDoc->SaveFile(Filename.c_str());
	}
	else
	{
		// If metadata is empty, remove existing meta file
		if (RFileUtil::CheckPathExists(Filename))
		{
			remove(Filename.c_str());
		}
	}
}

void RResourceMetaData::AddAttribute(const std::string& Key, const std::string& Value)
{
	Attributes[Key] = Value;
}

void RResourceMetaData::RemoveAttribute(const std::string& Key)
{
	auto Iter = Attributes.find(Key);
	if (Iter != Attributes.end())
	{
		Attributes.erase(Iter);
	}
}

const std::string& RResourceMetaData::operator[](const std::string& Key) const
{
	if (Attributes.find(Key) == Attributes.end())
	{
		static const std::string EmptyString = std::string();
		return EmptyString;
	}

	return Attributes.at(Key);
}
