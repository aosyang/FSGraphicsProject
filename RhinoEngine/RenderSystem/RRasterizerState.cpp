//=============================================================================
// RRasterizerState.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RRasterizerState.h"

#include "RRenderSystem.h"

// Lexicographical comparison operator. Allows D3D11_RASTERIZER_DESC to be used as a key of maps
bool operator<(const D3D11_RASTERIZER_DESC& Lhs, const D3D11_RASTERIZER_DESC& Rhs)
{
	return std::lexicographical_compare((char*)&Lhs, (char*)&Lhs + sizeof(Lhs), (char*)&Rhs, (char*)&Rhs + sizeof(Rhs));
}

RRasterizerState::RRasterizerState()
	: CurrentRasterizerDesc(CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT()))
{
	// Enable line antialiasing by default
	CurrentRasterizerDesc.AntialiasedLineEnable = true;
}

void RRasterizerState::SetDoubleSided(bool bDoubleSided)
{
	CurrentRasterizerDesc.CullMode = bDoubleSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;
}

void RRasterizerState::Apply()
{
	auto Iter = RasterizerStateMap.find(CurrentRasterizerDesc);
	if (Iter != RasterizerStateMap.end())
	{
		GRenderer.D3DImmediateContext()->RSSetState(Iter->second.Get());
	}
	else
	{
		ComPtr<ID3D11RasterizerState> RasterizerState;
		GRenderer.D3DDevice()->CreateRasterizerState(&CurrentRasterizerDesc, RasterizerState.GetAddressOf());
		GRenderer.D3DImmediateContext()->RSSetState(RasterizerState.Get());
		RasterizerStateMap[CurrentRasterizerDesc] = RasterizerState;
	}
}
