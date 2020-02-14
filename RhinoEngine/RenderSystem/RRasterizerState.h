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

	void SetDoubleSided(bool bDoubleSided);

	/// Apply current rasterizer state to the render pipeline
	void Apply();

private:
	D3D11_RASTERIZER_DESC CurrentRasterizerDesc;

	/// Mapping rasterizer descriptions to their objects
	std::map<D3D11_RASTERIZER_DESC, ComPtr<ID3D11RasterizerState>> RasterizerStateMap;
};
