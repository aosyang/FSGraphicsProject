//=============================================================================
// RAnimGraph.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimGraph.h"

RAnimGraph::RAnimGraph(const std::string& path)
	: RResourceBase(path)
	, RootNode(nullptr)
{

}

const std::vector<std::string>& RAnimGraph::GetSupportedExtensions()
{
	static const std::vector<std::string> AnimGraphExts{ ".ranimgraph" };
	return AnimGraphExts;
}

bool RAnimGraph::LoadResourceImpl()
{
	return true;
}

bool RAnimGraph::SaveResourceImpl()
{
	return true;
}
