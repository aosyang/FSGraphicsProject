//=============================================================================
// RAnimGraphEditor.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"


RAnimGraphEditor::RAnimGraphEditor()
	: bSetNodeInitialPosition(false)
{

}

void RAnimGraphEditor::ShowAnimGraphEditor()
{
	ImGui::Begin("AnimGraph Editor");
	{
		RAnimGraph* SelectedAnimGraph = nullptr;
		bool bSelectionChanged = false;

		// AnimGraph selection
		{
			static int IndexSelection = -1;
			static const char* LabelStrNone = "(None)";

			std::vector<RAnimGraph*> AnimGraphResources = RResourceManager::Instance().EnumerateResourcesOfType<RAnimGraph>();

			const char* Label = LabelStrNone;
			if (IndexSelection >= 0 && IndexSelection < (int)AnimGraphResources.size())
			{
				Label = AnimGraphResources[IndexSelection]->GetAssetPath().c_str();
			}

			if (ImGui::BeginCombo("Select AnimGraph", Label))
			{
				std::vector<RAnimGraph*> AnimGraphResources = RResourceManager::Instance().EnumerateResourcesOfType<RAnimGraph>();
				for (int i = 0; i < (int)AnimGraphResources.size(); i++)
				{
					if (ImGui::Selectable(AnimGraphResources[i]->GetAssetPath().c_str()))
					{
						if (IndexSelection != i)
						{
							bSelectionChanged = true;
						}

						IndexSelection = i;
					}
				}
				ImGui::EndCombo();
			}

			if (IndexSelection >= 0 && IndexSelection < (int)AnimGraphResources.size())
			{
				SelectedAnimGraph = AnimGraphResources[IndexSelection];
			}
		}

		if (SelectedAnimGraph)
		{
			if (ImGui::Button("Save"))
			{
				SaveAnimGraph(*SelectedAnimGraph);
			}

			if (bSelectionChanged)
			{
				BuildLinkGraph(*SelectedAnimGraph);
			}

			imnodes::BeginNodeEditor();
			ImNodesDrawOutputNode();
			for (auto& Node : SelectedAnimGraph->AnimGraphNodes)
			{
				ImNodesDrawAnimGraphNode(Node.get());
			}

			// Set node positions on their initial creation
			if (bSetNodeInitialPosition)
			{
				for (const auto& EditorNode : EditorNodes)
				{
					if (EditorNode.AnimGraphNode)
					{
						const auto& ChildEntries = EditorNode.AnimGraphNode->Attributes.ChildEntries;
						for (int i = 0; i < (int)ChildEntries.size(); i++)
						{
							if (ChildEntries[i].EntryName == "Editor")
							{
								float GridX = StringUtils::ToFloat(ChildEntries[i]["GridX"]);
								float GridY = StringUtils::ToFloat(ChildEntries[i]["GridY"]);
								imnodes::SetNodeGridSpacePos(EditorNode.NodeId, ImVec2(GridX, GridY));
							}
						}
					}
				}

				// Hack: Move graph output node next to the node it's connect to
				int InputNode = GetNodeIdByPinId(GetPinConnectedTo(EditorNodes[0].InputPinIds[0]));
				ImVec2 GridPos = imnodes::GetNodeGridSpacePos(InputNode);
				ImVec2 NodeSize = imnodes::GetNodeDimensions(InputNode);
				GridPos.x += NodeSize.x + 50.0f;
				imnodes::SetNodeGridSpacePos(EditorNodes[0].NodeId, GridPos);

				bSetNodeInitialPosition = false;
			}

			int link_id = 1;
			for (const auto& Iter : GraphLinks)
			{
				imnodes::Link(link_id++, Iter.first, Iter.second);
			}

			imnodes::EndNodeEditor();

			// Handle any new connections
			int start_attr, end_attr;
			if (imnodes::IsLinkCreated(&start_attr, &end_attr))
			{
				// Disconnect any other link to this pin if its an input pose
				DisconnectIfInputLink(start_attr);
				DisconnectIfInputLink(end_attr);
				GraphLinks.push_back(std::make_pair(start_attr, end_attr));

				// Modify connections for original nodes
				RAnimGraphNode* NodeWithInput = nullptr;
				RAnimGraphNode* NodeWithOutput = nullptr;
				int InputIndex = -1;
				if (GetInputPinById(start_attr, &NodeWithInput, &InputIndex))
				{
					NodeWithOutput = GetNodeByPinId(end_attr);
				}
				else
				{
					GetInputPinById(end_attr, &NodeWithInput, &InputIndex);
					NodeWithOutput = GetNodeByPinId(start_attr);
				}

				if (NodeWithOutput)
				{
					if (NodeWithInput)
					{
						NodeWithInput->Inputs[InputIndex] = NodeWithOutput->NodeName;
					}
					else
					{
						SelectedAnimGraph->RootGraphNode = NodeWithOutput;
					}
				}
			}
		}
	}
	ImGui::End();	// ImGui::Begin("AnimGraph Editor");
}

void RAnimGraphEditor::ImNodesDrawAnimGraphNode(RAnimGraphNode* Node)
{
	auto Iter = EditorNodes.begin();
	for (; Iter != EditorNodes.end(); Iter++)
	{
		if (Iter->AnimGraphNode == Node)
		{
			break;
		}
	}

	if (Iter == EditorNodes.end())
	{
		return;
	}

	imnodes::BeginNode(Iter->NodeId);

	imnodes::BeginNodeTitleBar();
	ImGui::Text(Node->NodeName.c_str());
	imnodes::EndNodeTitleBar();

	for (int i = 0; i < (int)Node->Inputs.size(); i++)
	{
		char InputName[32];
		sprintf_s(InputName, "Input %d", i);
		imnodes::BeginInputAttribute(Iter->InputPinIds[i]);
		ImGui::Text(InputName);
		imnodes::EndInputAttribute();
	}

	imnodes::BeginOutputAttribute(Iter->OutputPinId);
	// in between Begin|EndAttribute calls, you can call ImGui



	// UI functions
	ImGui::Text("Output");
	imnodes::EndOutputAttribute();

	imnodes::EndNode();
}

void RAnimGraphEditor::ImNodesDrawOutputNode()
{
	imnodes::BeginNode(EditorNodes[0].NodeId);

	imnodes::BeginNodeTitleBar();
	ImGui::Text("Graph Output");
	imnodes::EndNodeTitleBar();

	imnodes::BeginInputAttribute(EditorNodes[0].InputPinIds[0]);
	ImGui::Text("Pose");
	imnodes::EndInputAttribute();

	imnodes::EndNode();
}

void RAnimGraphEditor::BuildLinkGraph(const RAnimGraph& AnimGraph)
{
	GraphLinks.clear();
	EditorNodes.clear();

	int ui_id = 1;

	// Create an editor node for the final output. The anim graph node for it is null
	AnimGraphEditorNode RootNode;
	RootNode.AnimGraphNode = nullptr;
	RootNode.NodeId = ui_id++;
	RootNode.InputPinIds.push_back(ui_id++);
	RootNode.OutputPinId = -1;
	EditorNodes.push_back(RootNode);

	// Each node and pin need a unique ui id.
	for (auto& Node : AnimGraph.AnimGraphNodes)
	{
		AnimGraphEditorNode EditorNode;
		EditorNode.AnimGraphNode = Node.get();
		EditorNode.NodeId = ui_id++;
		for (int i = 0; i < RAnimGraph::GetNumInputPosesOfNodeType(Node->NodeTypeName); i++)
		{
			EditorNode.InputPinIds.push_back(ui_id++);
		}
		EditorNode.OutputPinId = ui_id++;
		EditorNodes.push_back(EditorNode);
	}

	// Now build the links
	for (auto& Node : AnimGraph.AnimGraphNodes)
	{
		for (int i = 0; i < (int)Node->Inputs.size(); i++)
		{
			const std::string& InputNodeName = Node->Inputs[i];
			AnimGraphEditorNode* OutputNode = GetEditorNodeByName(InputNodeName);
			AnimGraphEditorNode* ThisNode = GetEditorNodeByName(Node->NodeName);

			if (OutputNode && ThisNode)
			{
				GraphLinks.push_back(std::make_pair(ThisNode->InputPinIds[i], OutputNode->OutputPinId));
			}
		}
	}

	// Finally, connect output pose to the root node
	if (AnimGraph.RootGraphNode)
	{
		AnimGraphEditorNode* OutputNode = GetEditorNodeByName(AnimGraph.RootGraphNode->NodeName);
		GraphLinks.push_back(std::make_pair(EditorNodes[0].InputPinIds[0], OutputNode->OutputPinId));
	}

	bSetNodeInitialPosition = true;
}

void RAnimGraphEditor::SaveAnimGraph(RAnimGraph& SelectedAnimGraph)
{
	for (auto& GraphNode : SelectedAnimGraph.AnimGraphNodes)
	{
		AnimGraphEditorNode* EditorNode = GetEditorNodeByName(GraphNode->NodeName);
		ImVec2 PosInGrid = imnodes::GetNodeGridSpacePos(EditorNode->NodeId);

		AnimNodeAttributeMap::ChildEntry EditorEntry;
		EditorEntry.EntryName = "Editor";
		EditorEntry["GridX"] = std::to_string(PosInGrid.x);
		EditorEntry["GridY"] = std::to_string(PosInGrid.y);

		bool bEntryReplaced = false;
		auto& ChildEntries = GraphNode->Attributes.ChildEntries;
		for (int i = 0; i < (int)ChildEntries.size(); i++)
		{
			if (ChildEntries[i].EntryName == "Editor")
			{
				ChildEntries[i] = EditorEntry;
				bEntryReplaced = true;
				break;
			}
		}

		if (!bEntryReplaced)
		{
			ChildEntries.push_back(EditorEntry);
		}
	}

	SelectedAnimGraph.SaveToDisk();
}

void RAnimGraphEditor::DisconnectIfInputLink(int id)
{
	for (int i = 0; i < (int)EditorNodes.size(); i++)
	{
		for (int j = 0; j < (int)EditorNodes[i].InputPinIds.size(); j++)
		{
			// Make sure id is an input pin
			if (EditorNodes[i].InputPinIds[j] == id)
			{
				RemoveLinkById(id);

				if (EditorNodes[i].AnimGraphNode)
				{
					EditorNodes[i].AnimGraphNode->Inputs[j] = "";
				}

				return;
			}
		}
	}
}

void RAnimGraphEditor::RemoveLinkById(int id)
{
	GraphLinks.erase(
		std::remove_if(GraphLinks.begin(), GraphLinks.end(),
			[id](const std::pair<int, int>& Elem) {
				return Elem.first == id || Elem.second == id;
			}),
		GraphLinks.end()
	);
}

std::string RAnimGraphEditor::GetInputPinName(const RAnimGraphNode& Node, int Index) const
{
	return Node.NodeName + ":Input" + std::to_string(Index);
}

std::string RAnimGraphEditor::GetOutputPinName(const RAnimGraphNode& Node) const
{
	return Node.NodeName + ":Output";
}

int RAnimGraphEditor::GetPinConnectedTo(int id) const
{
	for (const auto& Link : GraphLinks)
	{
		if (Link.first == id)
		{
			return Link.second;
		}

		if (Link.second == id)
		{
			return Link.first;
		}
	}

	return -1;
}

RAnimGraphEditor::AnimGraphEditorNode* RAnimGraphEditor::GetEditorNodeByName(const std::string& NodeName)
{
	for (int i = 0; i < (int)EditorNodes.size(); i++)
	{
		if (RAnimGraphNode* Node = EditorNodes[i].AnimGraphNode)
		{
			if (Node->NodeName == NodeName)
			{
				return &EditorNodes[i];
			}
		}
	}

	return nullptr;
}

bool RAnimGraphEditor::GetInputPinById(int id, RAnimGraphNode** OutAnimGraphNode, int* OutInputIndex) const
{
	for (int i = 0; i < (int)EditorNodes.size(); i++)
	{
		for (int j = 0; j < (int)EditorNodes[i].InputPinIds.size(); j++)
		{
			// Make sure id is an input pin
			if (EditorNodes[i].InputPinIds[j] == id)
			{
				*OutAnimGraphNode = EditorNodes[i].AnimGraphNode;
				*OutInputIndex = j;
				return true;
			}
		}
	}

	return false;
}

RAnimGraphNode* RAnimGraphEditor::GetNodeByPinId(int id) const
{
	if (auto EditorNode = GetEditorNodeByPinId(id))
	{
		return EditorNode->AnimGraphNode;
	}

	return nullptr;
}

int RAnimGraphEditor::GetNodeIdByPinId(int id) const
{
	if (auto EditorNode = GetEditorNodeByPinId(id))
	{
		return EditorNode->NodeId;
	}

	return -1;
}

const RAnimGraphEditor::AnimGraphEditorNode* RAnimGraphEditor::GetEditorNodeByPinId(int id) const
{
	for (int i = 0; i < (int)EditorNodes.size(); i++)
	{
		// Match input pin with id
		for (int j = 0; j < (int)EditorNodes[i].InputPinIds.size(); j++)
		{
			if (EditorNodes[i].InputPinIds[j] == id)
			{
				return &EditorNodes[i];
			}
		}

		// Match output pin with id
		if (EditorNodes[i].OutputPinId == id)
		{
			return &EditorNodes[i];
		}
	}

	return nullptr;
}
