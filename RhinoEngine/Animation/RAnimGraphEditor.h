//=============================================================================
// RAnimGraphEditor.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RAnimGraphEditor
{
public:
	// Draw a GUI of anim graph editor
	void ShowAnimGraphEditor();

private:
	void ImNodesDrawAnimGraphNode(RAnimGraphNode* Node);
	void ImNodesDrawOutputNode();

	// Called every time when anim graph selection is changed
	void BuildLinkGraph(const RAnimGraph& AnimGraph);

	// Disconnect links to input pin with this ui id. (Won't change link if id is an output pin)
	void DisconnectIfInputLink(int id);

	void RemoveLinkById(int id);

	std::string GetInputPinName(const RAnimGraphNode& Node, int Index) const;
	std::string GetOutputPinName(const RAnimGraphNode& Node) const;

	std::vector<std::pair<int, int>> GraphLinks;

	//std::map<std::string, int> NodePinNameToUniqueId;

	// The node used for rendering with gui. This node holds ids for ui elements.
	struct AnimGraphEditorNode
	{
		RAnimGraphNode* AnimGraphNode;
		int NodeId;
		std::vector<int> InputIds;
		int OutputId;
	};

	AnimGraphEditorNode* GetEditorNodeByName(const std::string& NodeName);
	bool GetInputPinById(int id, RAnimGraphNode** OutAnimGraphNode, int* OutInputIndex) const;
	RAnimGraphNode* GetNodeByPinId(int id) const;

	std::vector<AnimGraphEditorNode> EditorNodes;
};
