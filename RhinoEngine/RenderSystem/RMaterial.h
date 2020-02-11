//=============================================================================
// RMaterial.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Resource/RResourceBase.h"

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

	/// Get a default material
	static RMaterial* GetDefault();

protected:
	virtual bool LoadResourceImpl() override;
	virtual bool SaveResourceImpl() override;

private:
	RShader* Shader;
	std::vector<RTextureSlotData> TextureSlots;
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
