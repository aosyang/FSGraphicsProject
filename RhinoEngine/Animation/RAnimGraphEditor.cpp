//=============================================================================
// RAnimGraphEditor.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

namespace
{
	void ImNodesDrawAnimGraphNode(RAnimGraphNode* Node, int& next_id)
	{
		imnodes::BeginNode(next_id++);

		imnodes::BeginNodeTitleBar();
		ImGui::Text(Node->NodeName.c_str());
		imnodes::EndNodeTitleBar();

		for (int i = 0; i < (int)Node->Inputs.size(); i++)
		{
			char InputName[32];
			sprintf_s(InputName, "Input %d", i);
			imnodes::BeginInputAttribute(next_id++);
			ImGui::Text(InputName);
			imnodes::EndInputAttribute();
		}

		imnodes::BeginOutputAttribute(next_id++);
		// in between Begin|EndAttribute calls, you can call ImGui



		// UI functions
		ImGui::Text("Output");
		imnodes::EndOutputAttribute();

		imnodes::EndNode();
	}

	void ImNodesDrawOutputNode(int& next_id)
	{
		imnodes::BeginNode(next_id++);

		imnodes::BeginNodeTitleBar();
		ImGui::Text("Output");
		imnodes::EndNodeTitleBar();

		imnodes::BeginInputAttribute(next_id++);
		ImGui::Text("Pose");
		imnodes::EndInputAttribute();

		imnodes::EndNode();
	}
}

void ShowAnimGraphEditor()
{
	ImGui::Begin("AnimGraph Editor");
	{
		RAnimGraph* SelectedAnimGraph = nullptr;

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
			int ui_id = 1;
			imnodes::BeginNodeEditor();
			ImNodesDrawOutputNode(ui_id);
			for (auto& Node : SelectedAnimGraph->AnimGraphNodes)
			{
				ImNodesDrawAnimGraphNode(Node.get(), ui_id);
			}
			imnodes::EndNodeEditor();
		}
	}
	ImGui::End();	// ImGui::Begin("AnimGraph Editor");
}
