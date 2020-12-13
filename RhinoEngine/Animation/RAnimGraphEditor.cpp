//=============================================================================
// RAnimGraphEditor.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"


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
				SelectedAnimGraph->SaveToDisk();
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
		imnodes::BeginInputAttribute(Iter->InputIds[i]);
		ImGui::Text(InputName);
		imnodes::EndInputAttribute();
	}

	imnodes::BeginOutputAttribute(Iter->OutputId);
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
	ImGui::Text("Output");
	imnodes::EndNodeTitleBar();

	imnodes::BeginInputAttribute(EditorNodes[0].InputIds[0]);
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
	RootNode.InputIds.push_back(ui_id++);
	RootNode.OutputId = -1;
	EditorNodes.push_back(RootNode);

	// Each node and pin need a unique ui id.
	for (auto& Node : AnimGraph.AnimGraphNodes)
	{
		AnimGraphEditorNode EditorNode;
		EditorNode.AnimGraphNode = Node.get();
		EditorNode.NodeId = ui_id++;
		for (int i = 0; i < RAnimGraph::GetNumInputPosesOfNodeType(Node->NodeTypeName); i++)
		{
			EditorNode.InputIds.push_back(ui_id++);
		}
		EditorNode.OutputId = ui_id++;
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
				GraphLinks.push_back(std::make_pair(ThisNode->InputIds[i], OutputNode->OutputId));
			}
		}
	}

	// Finally, connect output pose to the root node
	if (AnimGraph.RootGraphNode)
	{
		AnimGraphEditorNode* OutputNode = GetEditorNodeByName(AnimGraph.RootGraphNode->NodeName);
		GraphLinks.push_back(std::make_pair(EditorNodes[0].InputIds[0], OutputNode->OutputId));
	}
}

void RAnimGraphEditor::DisconnectIfInputLink(int id)
{
	for (int i = 0; i < (int)EditorNodes.size(); i++)
	{
		for (int j = 0; j < (int)EditorNodes[i].InputIds.size(); j++)
		{
			// Make sure id is an input pin
			if (EditorNodes[i].InputIds[j] == id)
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
		for (int j = 0; j < (int)EditorNodes[i].InputIds.size(); j++)
		{
			// Make sure id is an input pin
			if (EditorNodes[i].InputIds[j] == id)
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
	for (int i = 0; i < (int)EditorNodes.size(); i++)
	{
		if (EditorNodes[i].OutputId == id)
		{
			return EditorNodes[i].AnimGraphNode;
		}

		for (int j = 0; j < (int)EditorNodes[i].InputIds.size(); j++)
		{
			// Make sure id is an input pin
			if (EditorNodes[i].InputIds[j] == id)
			{
				return EditorNodes[i].AnimGraphNode;
			}
		}
	}

	return nullptr;
}
