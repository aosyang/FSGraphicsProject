//=============================================================================
// RAnimGraph.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Resource/RResourceBase.h"
#include "RAnimNode_Base.h"
#include "Core/StringUtils.h"


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

	// Type of node, in string
	std::string NodeTypeName;

	// Names of other nodes that output to this node. Note: Size of this array may differ from size of input poses
	std::vector<std::string> Inputs;
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
	static void RegisterAnimNodeType(const std::string& TypeName, AnimNodeFactoryMethod FactoryMethod, int NumInputPoses = 0);

	// Get how many input poses are accepted for given type name
	static int GetNumInputPosesOfNodeType(const std::string& TypeName);

protected:
	// Override RResourceBase methods
	virtual bool LoadResourceImpl() override;
	virtual bool SaveResourceImpl() override;

private:
	struct AnimNodeFactoryData
	{
		AnimNodeFactoryMethod Method;
		int NumInputPoses;
	};

	// Type name to factory method map for anim nodes
	static std::map<const std::string, AnimNodeFactoryData> AnimNodeFactoryMethods;

	static std::unique_ptr<RAnimNode_Base> CreateAnimNode(const std::string& NodeName, const std::string& TypeName, const AnimNodeAttributeMap& Attributes);
	static std::unique_ptr<RAnimNode_Base> CreateAnimNode(const RAnimGraphNode& AnimGraphNode);

	// Makes a unique name for node. Adds suffix '_#' if name already exists
	std::string MakeUniqueNodeName(const std::string& BaseName) const;

	RAnimGraphNode* FindGraphNodeByName(const std::string& Name) const;

	// Returns all nodes connected to the root node in an array (root node is included)
	std::vector<RAnimGraphNode*> CollectAllNodes() const;

	// Hack: Temporarily make members public to use them in anim graph editor
public:
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

	// Bind a pointer to an animation variable.
	// The variable name is in the format "NodeName:VariableName" in order to locate an input variable from any node in the graph.
	template<typename T>
	void BindAnimVariable(const std::string& NodeAndVariableName, T* ValuePtr);

private:
	// Find a node with given name in this anim graph instance
	RAnimNode_Base* FindNodeByName(const std::string& NodeName) const;

private:
	RAnimNode_Base* RootNode;

	// A list of all nodes in this graph instance for easy access.
	std::vector<std::unique_ptr<RAnimNode_Base>> Nodes;
};

template<typename T>
void RAnimGraphInstance::BindAnimVariable(const std::string& NodeAndVariableName, T* ValuePtr)
{
	auto Tokens = StringUtils::Split(NodeAndVariableName, ":");
	if (Tokens.size() >= 2)
	{
		const std::string& NodeName = Tokens[0];
		const std::string& VariableName = Tokens[1];

		if (RAnimNode_Base* Node = FindNodeByName(NodeName))
		{
			if (!Node->BindAnimVariable(VariableName, ValuePtr))
			{
				RLogWarning("Binding of anim variable '%s' has unrecognized anim variable name \'%s\'.\n", NodeAndVariableName.c_str(), VariableName.c_str());
			}

			return;
		}

		RLogWarning("Binding of anim variable '%s' didn't match any node with name \'%s\' in the graph.\n", NodeAndVariableName.c_str(), NodeName.c_str());
	}
	else
	{
		RLogWarning("Binding of anim variable '%s' has wrong number of tokens. The expected format is \'NodeName:VariableName\'.\n", NodeAndVariableName.c_str());
	}
}
