//=============================================================================
// RAnimGraph.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Resource/RResourceBase.h"

class RAnimNode;

class RAnimGraph : public RResourceBase
{
	DECLARE_RUNTIME_TYPE(RAnimGraph, RResourceBase)
public:
	RAnimGraph(const std::string& path);

	// Called by RResourceManager for registering a resource type with its extensions
	static const std::vector<std::string>& GetSupportedExtensions();

protected:
	// Override RResourceBase methods
	virtual bool LoadResourceImpl() override;
	virtual bool SaveResourceImpl() override;

private:
	RAnimNode* RootNode;
};
