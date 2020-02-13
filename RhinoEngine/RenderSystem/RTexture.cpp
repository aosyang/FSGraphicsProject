//=============================================================================
// RTexture.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RTexture.h"
#include "tinyxml2/tinyxml2.h"

const std::string RTexture::InternalTextureName("[Internal]");

RTexture::RTexture(const std::string& Path)
	: RResourceBase(Path)
	, m_SRV(nullptr)
	, m_Width(0)
	, m_Height(0)
	, bIsCubeMap(false)
	, bHasOwnershipOfResource(true)
{
}

RTexture::RTexture(ID3D11ShaderResourceView* ShaderResourceView, bool bTakeResourceOwnership)
	: RResourceBase(InternalTextureName)
	, m_SRV(ShaderResourceView)
	, m_Width(0)
	, m_Height(0)
	, bIsCubeMap(false)
	, bHasOwnershipOfResource(bTakeResourceOwnership)
{
	if (ShaderResourceView)
	{
		QueryTextureDesc(*ShaderResourceView);
	}
}

RTexture::~RTexture()
{
	Reset();
}

void RTexture::Reset()
{
	if (bHasOwnershipOfResource)
	{
		SAFE_RELEASE(m_SRV);
	}
	else
	{
		m_SRV = nullptr;
	}

	m_Width = 0;
	m_Height = 0;
	bIsCubeMap = false;
	bHasOwnershipOfResource = false;
}

std::vector<std::string> RTexture::GetSupportedExtensions()
{
	static const std::vector<std::string> TextureExts{ ".dds" };
	return TextureExts;
}

bool RTexture::LoadResourceImpl()
{
	size_t char_len;
	wchar_t wszName[1024];
	mbstowcs_s(&char_len, wszName, 1024, GetFileSystemPath().data(), GetFileSystemPath().size());

	RLog("Loading texture %s\n", GetAssetPath().c_str());

	bool bIsSRGBTexture = (GetMetaData()["SRGB"] == "true");
	HRESULT hr;
	ID3D11ShaderResourceView* ShaderResourceView;
	hr = DirectX::CreateDDSTextureFromFileEx(GRenderer.D3DDevice(), wszName, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, bIsSRGBTexture, nullptr, &ShaderResourceView);

	if (FAILED(hr))
	{
		RLog("*** Failed to load texture [%s] ***\n", GetFileSystemPath().data());
	}

	if (ShaderResourceView)
	{
		QueryTextureDesc(*ShaderResourceView);
	}

	m_SRV = ShaderResourceView;
	return true;
}

void RTexture::QueryTextureDesc(ID3D11ShaderResourceView& ShaderResourceView)
{
	ID3D11Resource* TextureResource;
	ShaderResourceView.GetResource(&TextureResource);
	if (TextureResource)
	{
		ID3D11Texture2D* pTexture;
		TextureResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);

		if (pTexture)
		{
			D3D11_TEXTURE2D_DESC Desc;
			pTexture->GetDesc(&Desc);

			m_Width = Desc.Width;
			m_Height = Desc.Height;
			bIsCubeMap = Desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE;

			pTexture->Release();
		}

		TextureResource->Release();
	}
}
