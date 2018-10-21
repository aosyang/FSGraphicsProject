//=============================================================================
// RTexture.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RTexture.h"
#include "tinyxml2/tinyxml2.h"

const std::string RTexture::InternalTextureName("[Internal]");

RTexture::RTexture(const string& Path)
	: RResourceBase(RT_Texture, Path), m_SRV(nullptr), m_Width(0), m_Height(0)
{
}

RTexture::RTexture(ID3D11ShaderResourceView* srv)
	: RResourceBase(RT_Texture, InternalTextureName), m_SRV(srv), m_Width(0), m_Height(0)
{
}

RTexture::~RTexture()
{
	SAFE_RELEASE(m_SRV);
}

bool RTexture::LoadResourceData(bool bIsAsyncLoading)
{
	ID3D11ShaderResourceView* srv;
	size_t char_len;
	wchar_t wszName[1024];
	mbstowcs_s(&char_len, wszName, 1024, GetPath().data(), GetPath().size());

	RLog("Loading texture [%s]...\n", GetPath().data());

	bool bIsSRGBTexture = false;

	unique_ptr<tinyxml2::XMLDocument> XmlDoc(new tinyxml2::XMLDocument());
	string MetaFileName = GetPath() + ".meta";
	if (XmlDoc->LoadFile(MetaFileName.c_str()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement* MetaElem = XmlDoc->FirstChildElement("Metadata");
		if (MetaElem)
		{
			static const char NameSRGB[] = "SRGB";
			MetaElem->QueryBoolAttribute(NameSRGB, &bIsSRGBTexture);
		}
	}

	ID3D11Resource* pResource;
	HRESULT hr;

	if (bIsSRGBTexture)
	{
		hr = DirectX::CreateDDSTextureFromFileEx(GRenderer.D3DDevice(), wszName, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, &pResource, &srv);
	}
	else
	{
		hr = DirectX::CreateDDSTextureFromFile(GRenderer.D3DDevice(), wszName, &pResource, &srv);
	}

	if (FAILED(hr))
	{
		RLog("*** Failed to load texture [%s] ***\n", GetPath().data());
	}

	if (pResource)
	{
		ID3D11Texture2D* pTexture;
		pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);

		if (pTexture)
		{
			D3D11_TEXTURE2D_DESC desc;
			pTexture->GetDesc(&desc);

			m_Width = desc.Width;
			m_Height = desc.Height;

			pTexture->Release();
		}

		pResource->Release();
	}

	m_SRV = srv;
	OnLoadingFinished(bIsAsyncLoading);

	return true;
}
