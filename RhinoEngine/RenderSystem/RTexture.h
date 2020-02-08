//=============================================================================
// RTexture.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RTexture : public RResourceBase
{
	DECLARE_RUNTIME_TYPE(RTexture, RResourceBase)
	friend class RResourceManager;
public:
	RTexture(const std::string& Path);

	/// Create texture from existing shader resource view
	RTexture(ID3D11ShaderResourceView* ShaderResourceView, bool bTakeResourceOwnership);
	~RTexture();

	/// Required by RResourceManager::RegisterResourceType
	static std::vector<std::string> GetSupportedExtensions();

	ID3D11ShaderResourceView* GetSRV() { return m_SRV; }
	ID3D11ShaderResourceView** GetPtrSRV() { return &m_SRV; }
	UINT GetWidth() const { return m_Width; }
	UINT GetHeight() const { return m_Height; }
	bool IsCubeMap() const { return bIsCubeMap; }

	/// Check if RTexture owns its hardware texture
	bool HasOwnershipOfResource() const;

protected:

	virtual bool LoadResourceImpl(bool bIsAsyncLoading) override;
	
private:
	void QueryTextureDesc(ID3D11ShaderResourceView& ShaderResourceView);

private:
	ID3D11ShaderResourceView*	m_SRV;
	UINT						m_Width, m_Height;
	bool	bIsCubeMap;

	/// If a RTexture has ownership of a hardware resource, the RTexture is responsible for releasing the hardware resource on destruction
	bool	bHasOwnershipOfResource;

	static const std::string			InternalTextureName;
};

FORCEINLINE bool RTexture::HasOwnershipOfResource() const
{
	return bHasOwnershipOfResource;
}

