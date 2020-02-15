//=============================================================================
// RRasterizerState.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"
#include "D3DCommonPrivate.h"

class RRasterizerState
{
public:
	RRasterizerState();

	/// Create a default rasterizer descriptor
	D3D11_RASTERIZER_DESC MakeDefaultDescriptor();

	/// Apply current rasterizer state to the render pipeline
	void Apply(size_t RasterizerStateHash);
	
	/// Find an existing hash value for the rasterizer descriptor. If it doesn't exist, then create a new one.
	size_t FindOrAddRasterizerStateHash(const D3D11_RASTERIZER_DESC& Desc);

private:
	struct RasterizerStateData
	{
		RasterizerStateData() = default;
		RasterizerStateData(ComPtr<ID3D11RasterizerState> InRasterizerStateObject, const D3D11_RASTERIZER_DESC& InDesc)
			: RasterizerStateObject(InRasterizerStateObject)
			, Desc(InDesc)
		{
		}

		RasterizerStateData& operator=(const RasterizerStateData& Other)
		{
			RasterizerStateObject = Other.RasterizerStateObject;
			Desc = Other.Desc;
			return *this;
		}

		ComPtr<ID3D11RasterizerState> RasterizerStateObject;
		D3D11_RASTERIZER_DESC Desc;
	};

	/// Mapping rasterizer state hashes to rasterizer state objects
	std::map<size_t, RasterizerStateData> RasterizerStateMap;
};
