//=============================================================================
// RResourceMetaData.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RResourceMetaData.h"
#include "tinyxml2/tinyxml2.h"

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

const std::string& RResourceMetaData::operator[](const std::string& Key) const
{
	if (Attributes.find(Key) == Attributes.end())
	{
		static const std::string EmptyString = std::string();
		return EmptyString;
	}

	return Attributes.at(Key);
}
