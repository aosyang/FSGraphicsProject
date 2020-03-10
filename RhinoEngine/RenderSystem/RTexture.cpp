//=============================================================================
// RTexture.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RTexture.h"
#include "D3DCommonPrivate.h"
#include "tinyxml2/tinyxml2.h"
#include "hdrloader.h"

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
	static const std::vector<std::string> TextureExts{ ".dds", ".hdr" };
	return TextureExts;
}

bool RTexture::LoadResourceImpl()
{
	RLog("Loading texture %s\n", GetAssetPath().c_str());
	bool bIsSRGBTexture = (GetMetaData()["SRGB"] == "true");

	std::string Ext = RFileUtil::GetExtensionInLowerCase(GetAssetPath());
	if (Ext == "dds")
	{
		return LoadTextureDDS(bIsSRGBTexture);
	}
	else if (Ext == "hdr")
	{
		return LoadTextureHDR(bIsSRGBTexture);
	}

	return false;
}

bool RTexture::LoadTextureDDS(bool bSRGB)
{
	size_t char_len;
	wchar_t wszName[1024];
	mbstowcs_s(&char_len, wszName, 1024, GetFileSystemPath().data(), GetFileSystemPath().size());

	HRESULT hr;
	ID3D11ShaderResourceView* ShaderResourceView;
	hr = DirectX::CreateDDSTextureFromFileEx(GRenderer.D3DDevice(), wszName, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, bSRGB, nullptr, &ShaderResourceView);

	if (FAILED(hr))
	{
		RLog("*** Failed to load texture [%s] ***\n", GetFileSystemPath().data());
		return false;
	}

	if (ShaderResourceView)
	{
		QueryTextureDesc(*ShaderResourceView);
	}

	m_SRV = ShaderResourceView;
	return true;
}

bool RTexture::LoadTextureHDR(bool bSRGB)
{
	HDRLoaderResult Result;
	if (HDRLoader::load(GetFileSystemPath().data(), Result))
	{
		UINT SupportFlags;
		if (FAILED(GRenderer.D3DDevice()->CheckFormatSupport(DXGI_FORMAT_R32G32B32A32_FLOAT, &SupportFlags)))
		{
			return false;
		}

		if ((SupportFlags & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE) == 0)
		{
			RLogError("Unabled to load RGBE texture: Sampler for R32G32B32A32 is not supported by the hardward!\n");
			return false;
		}

		D3D11_TEXTURE2D_DESC TextureDesc;
		TextureDesc.Width = Result.width;
		TextureDesc.Height = Result.height;
		TextureDesc.MipLevels = TextureDesc.ArraySize = 1;
		TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.Usage = D3D11_USAGE_DEFAULT;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDesc.CPUAccessFlags = 0;
		TextureDesc.MiscFlags = 0;

		std::vector<float> Pixel;
		Pixel.resize(Result.width * Result.height * 4);
		for (int i = 0; i < Result.width * Result.height; i++)
		{
			Pixel[i * 4] = Result.cols[i * 3];
			Pixel[i * 4 + 1] = Result.cols[i * 3 + 1];
			Pixel[i * 4 + 2] = Result.cols[i * 3 + 2];
			Pixel[i * 4 + 3] = 1.0f;
		}

		D3D11_SUBRESOURCE_DATA Data;
		Data.pSysMem = Pixel.data();
		Data.SysMemPitch = Result.width * 4 * sizeof(float);
		Data.SysMemSlicePitch = 0;

		ComPtr<ID3D11Texture2D> pTexture;
		if (FAILED(GRenderer.D3DDevice()->CreateTexture2D(&TextureDesc, &Data, &pTexture)))
		{
			RLogError("Failed to create texture 2d resource while loading %s!\n", GetAssetPath().c_str());
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc;
		SrvDesc.Format = TextureDesc.Format;
		SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SrvDesc.Texture2D.MostDetailedMip = 0;
		SrvDesc.Texture2D.MipLevels = -1;

		ID3D11ShaderResourceView* ShaderResourceView;
		if (FAILED(GRenderer.D3DDevice()->CreateShaderResourceView(pTexture.Get(), &SrvDesc, &ShaderResourceView)))
		{
			RLogError("Failed to create shader resource view while loading %s!\n", GetAssetPath().c_str());
			return false;
		}

#if _DEBUG
		ShaderResourceView->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(GetAssetPath().c_str()), GetAssetPath().c_str());
#endif // _DEBUG

		m_SRV = ShaderResourceView;
		return true;
	}

	return false;
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
