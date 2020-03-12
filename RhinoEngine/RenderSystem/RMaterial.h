//=============================================================================
// RMaterial.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Resource/RResourceBase.h"
#include "BlendState.h"

struct RShader;
class RTexture;
class RSerializer;

// Material data used by each mesh element (obsoleted, replace this by RMaterial)
struct RMeshMaterialData
{
	RShader*					Shader;
	int							TextureNum;		/// Texture numbers, max 8
	RTexture*					Textures[8];

	/// Binary serialization operation
	void Serialize(RSerializer& serializer);

	/// Load an array of materials from xml file
	static std::vector<std::string> LoadFromXmlFile(const std::string& Filename);
};

struct RTextureSlotData
{
	RTextureSlotData()
		: Texture(nullptr)
		, SlotId(-1)
	{}

	RTextureSlotData(RTexture* InTexture, int InSlotId)
		: Texture(InTexture)
		, SlotId(InSlotId)
	{}

	void Serialize(RSerializer& Serializer);

	std::string GetTextureAssetPath() const;

	RTexture* Texture;
	int SlotId;
};

/// The material asset class. Managed by the resource manager and used for rendering
class RMaterial : public RResourceBase
{
	DECLARE_RUNTIME_TYPE(RMaterial, RResourceBase)
public:
	RMaterial(const std::string& Path);

	/// Required by RResourceManager::RegisterResourceType
	static std::vector<std::string> GetSupportedExtensions();

	/// Binary serialization operation
	void Serialize(RSerializer& serializer);

	void SetShader(RShader* InShader);
	RShader* GetShader() const;

	const std::vector<RTextureSlotData>& GetTextureSlots() const;
	std::vector<RTextureSlotData>& GetTextureSlots();

	void SetTextureSlot(int Slot, RTexture* Texture);
	RTexture* GetTextureBySlot(int SlotId) const;
	void RemoveTextureSlot(int SlotId);

	BlendState GetBlendMode() const;
	void SetBlendMode(BlendState InBlendMode);

	/// Number of times texture will be repeated when applying to a surface
	float GetUVTiling() const;

	bool GetDoubleSided() const;
	void SetDoubleSided(bool InDoubleSided);

	/// Get a default material
	static RMaterial* GetDefault();

	/// Get a depth-only material for rendering shadow passes
	static RMaterial* GetDepthOnly();

	/// Rasterizer state hash functions
	bool IsRasterizerStateHashOutOfDate() const;
	void SetRasterizerStateHash(size_t NewHash);
	size_t GetRasterizerStateHash() const;

protected:
	virtual bool LoadResourceImpl() override;
	virtual bool SaveResourceImpl() override;

private:
	RShader* Shader;
	std::vector<RTextureSlotData> TextureSlots;
	BlendState BlendMode;
	bool bDoubleSided;
	float UVTiling;

	bool bRasterizerStateHashOutOfDate;
	size_t RasterizerStateHash;

	static const char* KeyName_BlendMode;
	static const char* KeyName_UVTiling;
};

FORCEINLINE void RMaterial::SetShader(RShader* InShader)
{
	Shader = InShader;
}

FORCEINLINE RShader* RMaterial::GetShader() const
{
	return Shader;
}

FORCEINLINE const std::vector<RTextureSlotData>& RMaterial::GetTextureSlots() const
{
	return TextureSlots;
}

FORCEINLINE std::vector<RTextureSlotData>& RMaterial::GetTextureSlots()
{
	return TextureSlots;
}

FORCEINLINE RTexture* RMaterial::GetTextureBySlot(int SlotId) const
{
	for (int i = 0; i < TextureSlots.size(); i++)
	{
		if (TextureSlots[i].SlotId == SlotId)
		{
			return TextureSlots[i].Texture;
		}
	}

	return nullptr;
}

FORCEINLINE void RMaterial::RemoveTextureSlot(int SlotId)
{
	for (int i = 0; i < TextureSlots.size(); i++)
	{
		if (TextureSlots[i].SlotId == SlotId)
		{
			TextureSlots.erase(TextureSlots.begin() + i);
			break;
		}
	}
}

FORCEINLINE BlendState RMaterial::GetBlendMode() const
{
	return BlendMode;
}

FORCEINLINE void RMaterial::SetBlendMode(BlendState InBlendMode)
{
	BlendMode = InBlendMode;
}

FORCEINLINE float RMaterial::GetUVTiling() const
{
	return UVTiling;
}

FORCEINLINE bool RMaterial::GetDoubleSided() const
{
	return bDoubleSided;
}

FORCEINLINE void RMaterial::SetDoubleSided(bool InDoubleSided)
{
	bDoubleSided = InDoubleSided;
	bRasterizerStateHashOutOfDate = true;
}

FORCEINLINE bool RMaterial::IsRasterizerStateHashOutOfDate() const
{
	return bRasterizerStateHashOutOfDate;
}

FORCEINLINE void RMaterial::SetRasterizerStateHash(size_t NewHash)
{
	RasterizerStateHash = NewHash;
}

FORCEINLINE size_t RMaterial::GetRasterizerStateHash() const
{
	return RasterizerStateHash;
}
