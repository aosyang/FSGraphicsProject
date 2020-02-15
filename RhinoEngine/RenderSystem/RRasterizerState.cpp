//=============================================================================
// RRasterizerState.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RRasterizerState.h"

#include "RRenderSystem.h"

bool operator==(const D3D11_RASTERIZER_DESC& Lhs, const D3D11_RASTERIZER_DESC& Rhs)
{
	return Lhs.FillMode == Rhs.FillMode &&
		Lhs.CullMode == Rhs.CullMode &&
		Lhs.FrontCounterClockwise == Rhs.FrontCounterClockwise &&
		Lhs.DepthBias == Rhs.DepthBias &&
		Lhs.DepthBiasClamp == Rhs.DepthBiasClamp &&
		Lhs.SlopeScaledDepthBias == Rhs.SlopeScaledDepthBias &&
		Lhs.DepthClipEnable == Rhs.DepthClipEnable &&
		Lhs.ScissorEnable == Rhs.ScissorEnable &&
		Lhs.MultisampleEnable == Rhs.MultisampleEnable &&
		Lhs.AntialiasedLineEnable == Rhs.AntialiasedLineEnable;
}

size_t RasterizerDescription_CalcHash(const D3D11_RASTERIZER_DESC& Val)
{
	size_t Hash = 0;
	HashCombine(Hash, Val.FillMode);
	HashCombine(Hash, Val.CullMode);
	HashCombine(Hash, Val.FrontCounterClockwise);
	HashCombine(Hash, Val.DepthBias);
	HashCombine(Hash, Val.DepthBiasClamp);
	HashCombine(Hash, Val.SlopeScaledDepthBias);
	HashCombine(Hash, Val.DepthClipEnable);
	HashCombine(Hash, Val.ScissorEnable);
	HashCombine(Hash, Val.MultisampleEnable);
	HashCombine(Hash, Val.AntialiasedLineEnable);

	return Hash;
}


RRasterizerState::RRasterizerState()
{
}

D3D11_RASTERIZER_DESC RRasterizerState::MakeDefaultDescriptor()
{
	D3D11_RASTERIZER_DESC DefaultDescriptor = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
	DefaultDescriptor.AntialiasedLineEnable = true;
	return DefaultDescriptor;
}

void RRasterizerState::Apply(size_t RasterizerStateHash)
{
	auto Iter = RasterizerStateMap.find(RasterizerStateHash);
	if (Iter != RasterizerStateMap.end())
	{
		GRenderer.D3DImmediateContext()->RSSetState(Iter->second.RasterizerStateObject.Get());
	}
	else
	{
		RLogWarning("Couldn't find a rasterizer state object for hash %zu\n", RasterizerStateHash);
	}
}

size_t RRasterizerState::FindOrAddRasterizerStateHash(const D3D11_RASTERIZER_DESC& Desc)
{
	size_t Hash = RasterizerDescription_CalcHash(Desc);

	auto Iter = RasterizerStateMap.find(Hash);
	if (Iter != RasterizerStateMap.end())
	{
		// Verifies if rasterizer state descriptions are the same. If not, it means a hash collision happens and we need a better hash function
		assert(Iter->second.Desc == Desc);

		return Iter->first;
	}

	ComPtr<ID3D11RasterizerState> RasterizerState;
	GRenderer.D3DDevice()->CreateRasterizerState(&Desc, RasterizerState.GetAddressOf());
	RasterizerStateMap[Hash] = RasterizerStateData(RasterizerState, Desc);

	return Hash;
}
