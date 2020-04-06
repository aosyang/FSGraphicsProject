//=============================================================================
// RTexture.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Resource/RResourceBase.h"

struct ID3D11ShaderResourceView;

class RTexture : public RResourceBase
{
	DECLARE_RUNTIME_TYPE(RTexture, RResourceBase)
	friend class RResourceManager;
public:
	RTexture(const std::string& Path);

	/// Create texture from existing shader resource view
	RTexture(ID3D11ShaderResourceView* ShaderResourceView, bool bTakeResourceOwnership);
	virtual ~RTexture();

	virtual void Reset() override;

	/// Required by RResourceManager::RegisterResourceType
	static std::vector<std::string> GetSupportedExtensions();

	ID3D11ShaderResourceView* GetSRV() { assert(m_SRV); return m_SRV; }
	ID3D11ShaderResourceView** GetPtrSRV() { return &m_SRV; }
	UINT GetWidth() const { return m_Width; }
	UINT GetHeight() const { return m_Height; }
	UINT GetMipLevels() const { return MipLevels; }
	bool IsCubeMap() const { return bIsCubeMap; }

	/// Check if RTexture owns its hardware texture
	bool HasOwnershipOfResource() const;

protected:

	virtual bool LoadResourceImpl() override;

	/// Load a DDS texture
	bool LoadTextureDDS(bool bSRGB);

	/// Load a HDR texture
	bool LoadTextureHDR(bool bSRGB);
	
private:
	void QueryTextureDesc(ID3D11ShaderResourceView& ShaderResourceView);

private:
	ID3D11ShaderResourceView*	m_SRV;
	UINT						m_Width, m_Height;
	UINT	MipLevels;
	bool	bIsCubeMap;

	/// If a RTexture has ownership of a hardware resource, the RTexture is responsible for releasing the hardware resource on destruction
	bool	bHasOwnershipOfResource;

	static const std::string			InternalTextureName;
};

FORCEINLINE bool RTexture::HasOwnershipOfResource() const
{
	return bHasOwnershipOfResource;
}

