//=============================================================================
// RAnimGraph.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Resource/RResourceBase.h"
#include "RAnimNode_Base.h"


class RAnimGraphNode;
class RAnimGraphInstance;

class RAnimGraphNode
{
public:
	RAnimGraphNode(const std::string& InNodeName, const std::string& InNodeTypeName)
		: NodeName(InNodeName)
		, NodeTypeName(InNodeTypeName)
	{}

	std::string NodeName;
	std::string NodeTypeName;
	std::vector<RAnimGraphNode*> Inputs;
	AnimNodeAttributeMap Attributes;
};


// Signature of factory method for creating anim nodes
typedef std::unique_ptr<RAnimNode_Base> (*AnimNodeFactoryMethod)(const std::string&, const AnimNodeAttributeMap&);


// The animation graph is a resource of a template for runtime animation evaluation
class RAnimGraph : public RResourceBase
{
	DECLARE_RUNTIME_TYPE(RAnimGraph, RResourceBase)
public:
	RAnimGraph(const std::string& path);

	// Called by RResourceManager for registering a resource type with its extensions
	static const std::vector<std::string>& GetSupportedExtensions();

	// Create a instance of this animation graph
	std::shared_ptr<RAnimGraphInstance> CreateInstance();

	// Make an new node and bind it as the input slot of a given node.
	// If Node is null, the new animation node will be added as a root node.
	RAnimGraphNode* AddInputAnimNode(const std::string& NodeTypeName, const std::string& InNodeName = "", RAnimGraphNode* BaseNode = nullptr, int InputIndex = 0);

	static void RegisterAnimNodeTypes();

	// Register a type of anim node by its factory method to the anim node graph
	static void RegisterAnimNodeType(const std::string& TypeName, AnimNodeFactoryMethod FactoryMethod);

protected:
	// Override RResourceBase methods
	virtual bool LoadResourceImpl() override;
	virtual bool SaveResourceImpl() override;

private:
	// Type name to factory method map for anim nodes
	static std::map<const std::string, AnimNodeFactoryMethod> AnimNodeFactoryMethods;

	static std::unique_ptr<RAnimNode_Base> CreateAnimNode(const std::string& NodeName, const std::string& TypeName, const AnimNodeAttributeMap& Attributes);
	static std::unique_ptr<RAnimNode_Base> CreateAnimNode(const RAnimGraphNode& AnimGraphNode);

	// Makes a unique name for node. Adds suffix '_#' if name already exists
	std::string MakeUniqueNodeName(const std::string& BaseName) const;

	// List of all template nodes to be created from this graph
	std::vector<std::unique_ptr<RAnimGraphNode>> AnimGraphNodes;

	// The root node of this graph
	RAnimGraphNode* RootGraphNode;
};


// The instance of animation graph. Contains a copy of data for updating a single skinned mesh
class RAnimGraphInstance
{
	friend class RAnimGraph;
public:
	RAnimGraphInstance();

	void Update(float DeltaTime);
	void EvaluatePose(RAnimPoseData& PoseData);

	void BindAnimVariable(const std::string& VariableName, float* ValPtr);

private:
	std::unique_ptr<RAnimNode_Base> RootNode;

	// A list of all nodes in this graph instance for easy access.
	std::vector<RAnimNode_Base*> Nodes;
};

