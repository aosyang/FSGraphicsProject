//=============================================================================
// ILight.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "ILight.h"
#include "tinyxml2/tinyxml2.h"

RLight::RLight(RSceneObject* InOwner) 
	: Base(InOwner)
	, LightColor(1.0f, 1.0f, 1.0f, 1.0f)
	, LightIntensity(1.0f)
{
}

ELightType RLight::GetLightType() const
{
	return ELightType::Unspecific;
}

RAabb RLight::GetEffectiveLightBounds()
{
	return RAabb::Default;
}

void RLight::SetupConstantBuffer(int LightIndex) const
{

}

void RLight::SetLightColor(const RColor& NewColor)
{
	LightColor = NewColor;
}

const RColor& RLight::GetLightColor() const
{
	return LightColor;
}

void RLight::SetLightIntensity(float NewIntensity)
{
	LightIntensity = NewIntensity;
}

float RLight::GetLightIntensity() const
{
	return LightIntensity;
}

void RLight::LoadFromXmlElement(tinyxml2::XMLElement* ComponentElem)
{
	const char* ColorValue = ComponentElem->Attribute("Color");
	if (ColorValue)
	{
		std::stringstream StringStream(ColorValue);
		RColor LightColor;
		for (int n = 0; n < 3; n++)
		{
			StringStream >> LightColor[n];

			// Ignore remaining commas in the input string
			if (StringStream.peek() == ',')
			{
				StringStream.ignore();
			}
		}

		SetLightColor(LightColor);
	}

	float LightIntensity;
	if (ComponentElem->QueryFloatAttribute("Intensity", &LightIntensity) == tinyxml2::XML_SUCCESS)
	{
		SetLightIntensity(LightIntensity);
	}
}

void RLight::SaveToXmlElement(tinyxml2::XMLElement* ComponentElem) const
{
	std::stringstream StringStream;
	const RColor LightColor = GetLightColor();
	StringStream << LightColor.r << ", " << LightColor.g << ", " << LightColor.b;
	ComponentElem->SetAttribute("Color", StringStream.str().c_str());
	ComponentElem->SetAttribute("Intensity", GetLightIntensity());
}
