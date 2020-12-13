//=============================================================================
// RAnimGraph.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RAnimGraph.h"

#include "tinyxml2/tinyxml2.h"
#include "Core/StdHelper.h"
#include "Core/RLog.h"

#include "RAnimNode_AnimationPlayer.h"
#include "RAnimNode_BlendPlayer.h"
#include "RAnimNode_ModifyBoneTransform.h"
#include "Core/StringUtils.h"


std::map<const std::string, RAnimGraph::AnimNodeFactoryData> RAnimGraph::AnimNodeFactoryMethods;


RAnimGraph::RAnimGraph(const std::string& path)
	: RResourceBase(path)
	, RootGraphNode(nullptr)
{

}

const std::vector<std::string>& RAnimGraph::GetSupportedExtensions()
{
	static const std::vector<std::string> AnimGraphExts{ ".ranimgraph" };
	return AnimGraphExts;
}

std::shared_ptr<RAnimGraphInstance> RAnimGraph::CreateInstance()
{
	std::shared_ptr<RAnimGraphInstance> AnimGraphInstance = std::make_shared<RAnimGraphInstance>();
	RAnimGraphInstance* RawAnimGraphInstance = AnimGraphInstance.get();

	// 1. Create all nodes for this anim graph instance
	// 2. Connect nodes

	std::vector<RAnimGraphNode*> GraphNodes = CollectAllNodes();
	for (auto GraphNode : GraphNodes)
	{
		std::unique_ptr<RAnimNode_Base> NewNode = CreateAnimNode(*GraphNode);
		if (GraphNode == RootGraphNode)
		{
			RawAnimGraphInstance->RootNode = NewNode.get();
		}
		RawAnimGraphInstance->Nodes.push_back(std::move(NewNode));
	}

	for (int i = 0; i < (int)GraphNodes.size(); i++)
	{
		int NumInputs = (int)GraphNodes[i]->Inputs.size();
		RawAnimGraphInstance->Nodes[i]->InputPoses.resize(NumInputs);

		for (int j = 0; j < NumInputs; j++)
		{
			RawAnimGraphInstance->Nodes[i]->InputPoses[j] = RawAnimGraphInstance->FindNodeByName(GraphNodes[i]->Inputs[j]);
		}
	}

	return AnimGraphInstance;
}

RAnimGraphNode* RAnimGraph::AddInputAnimNode(const std::string& NodeTypeName, const std::string& InNodeName /*= ""*/, RAnimGraphNode* BaseNode /*= nullptr*/, int InputIndex /*= 0*/)
{
	const std::string NewNodeName = MakeUniqueNodeName(InNodeName == "" ? NodeTypeName : InNodeName);

	// Create a instance of new anim graph node
	auto NewNode = std::make_unique<RAnimGraphNode>(NewNodeName, NodeTypeName);

	if (BaseNode)
	{
		// Resize node inputs if necessary
		if (InputIndex >= BaseNode->Inputs.size())
		{
			BaseNode->Inputs.resize(InputIndex + 1);
		}
		BaseNode->Inputs[InputIndex] = NewNode->NodeName;
	}
	else
	{
		// Assign new node as root if no node is specified
		assert(RootGraphNode == nullptr);
		RootGraphNode = NewNode.get();
	}

	AnimGraphNodes.push_back(std::move(NewNode));
	return AnimGraphNodes.back().get();
}

void RAnimGraph::RegisterAnimNodeTypes()
{
	RAnimGraph::RegisterAnimNodeType("AnimationPlayer", &RAnimNode_AnimationPlayer::FactoryCreate);
	RAnimGraph::RegisterAnimNodeType("BlendPlayer", &RAnimNode_BlendPlayer::FactoryCreate);
	RAnimGraph::RegisterAnimNodeType("ModifyBoneTransform", &RAnimNode_ModifyBoneTransform::FactoryCreate, 1);
}

void RAnimGraph::RegisterAnimNodeType(const std::string& TypeName, AnimNodeFactoryMethod FactoryMethod, int NumInputPoses)
{
	AnimNodeFactoryMethods[TypeName] = AnimNodeFactoryData{ FactoryMethod, NumInputPoses };
}

bool RAnimGraph::LoadResourceImpl()
{
	std::unique_ptr<tinyxml2::XMLDocument> XmlDoc = std::make_unique<tinyxml2::XMLDocument>();
	if (XmlDoc->LoadFile(GetFileSystemPath().c_str()) == tinyxml2::XML_SUCCESS)
	{
		// <AnimGraph>
		tinyxml2::XMLElement* XmlElemAnimGraph = XmlDoc->RootElement();
		if (XmlElemAnimGraph)
		{
			tinyxml2::XMLElement* XmlElemAnimGraphChild = XmlElemAnimGraph->FirstChildElement();
			while (XmlElemAnimGraphChild)
			{
				if (!strcmp(XmlElemAnimGraphChild->Name(), "Nodes"))
				{
					tinyxml2::XMLElement* XmlElemNodes = XmlElemAnimGraphChild;
					tinyxml2::XMLElement* XmlElemNode = XmlElemNodes->FirstChildElement();
					while (XmlElemNode)
					{
						std::string NodeName, NodeTypeName;
						AnimNodeAttributeMap AttributeMap;
						const tinyxml2::XMLAttribute* XmlAnimNodeAttribute = XmlElemNode->FirstAttribute();

						// Read animation node
						while (XmlAnimNodeAttribute)
						{
							if (!strcmp(XmlAnimNodeAttribute->Name(), "Name"))
							{
								NodeName = XmlAnimNodeAttribute->Value();
							}
							else if (!strcmp(XmlAnimNodeAttribute->Name(), "Type"))
							{
								NodeTypeName = XmlAnimNodeAttribute->Value();
							}
							else
							{
								// Store other attributes into map
								AttributeMap[XmlAnimNodeAttribute->Name()] = XmlAnimNodeAttribute->Value();
							}

							XmlAnimNodeAttribute = XmlAnimNodeAttribute->Next();
						}

						std::vector<std::string> InputNodeNames;

						// Read child entries
						const tinyxml2::XMLElement* XmlElemChildEntry = XmlElemNode->FirstChildElement();
						while (XmlElemChildEntry)
						{
							if (StringUtils::EqualsIgnoreCase(XmlElemChildEntry->Name(), "Input"))
							{
								std::string NodeName = XmlElemChildEntry->Attribute("NodeName");
								int Index = -1;
								XmlElemChildEntry->QueryIntAttribute("Index", &Index);

								if (Index != -1)
								{
									if ((int)InputNodeNames.size() <= Index)
									{
										InputNodeNames.resize(Index + 1);
									}

									InputNodeNames[Index] = NodeName;
								}
							}
							else
							{
								AnimNodeAttributeMap::ChildEntry Entry;

								// Node name is the entry name
								Entry.EntryName = XmlElemChildEntry->Name();

								// Collect all attributes for entry
								const tinyxml2::XMLAttribute* XmlChildEntryAttribute = XmlElemChildEntry->FirstAttribute();
								while (XmlChildEntryAttribute)
								{
									Entry.Map[XmlChildEntryAttribute->Name()] = XmlChildEntryAttribute->Value();
									XmlChildEntryAttribute = XmlChildEntryAttribute->Next();
								}
								AttributeMap.ChildEntries.push_back(std::move(Entry));
							}
							XmlElemChildEntry = XmlElemChildEntry->NextSiblingElement();
						}

						auto NewNode = std::make_unique<RAnimGraphNode>(NodeName, NodeTypeName);
						NewNode->Attributes = std::move(AttributeMap);
						NewNode->Inputs = InputNodeNames;

						AnimGraphNodes.push_back(std::move(NewNode));
						XmlElemNode = XmlElemNode->NextSiblingElement();
					}
				}
				else if (!strcmp(XmlElemAnimGraphChild->Name(), "RootNode"))
				{
					// Search all graph nodes and find one that matches the name from 'RootNode'
					std::string RootNodeName = XmlElemAnimGraphChild->Attribute("Name");
					for (auto& Iter : AnimGraphNodes)
					{
						if (Iter.get()->NodeName == RootNodeName)
						{
							RootGraphNode = Iter.get();
							break;
						}
					}
				}

				XmlElemAnimGraphChild = XmlElemAnimGraphChild->NextSiblingElement();
			}

			return true;
		}
	}

	return false;
}

bool RAnimGraph::SaveResourceImpl()
{
	std::unique_ptr<tinyxml2::XMLDocument> XmlDoc = std::make_unique<tinyxml2::XMLDocument>();

	// Save asset path in header comments
	XmlDoc->InsertEndChild(XmlDoc->NewComment((std::string("AnimGraph path: ") + GetAssetPath()).c_str()));
	{
		// <AnimGraph>
		tinyxml2::XMLElement* XmlElemAnimGraph = XmlDoc->NewElement("AnimGraph");
		{
			// <Nodes>
			{
				// All nodes of this anim graph
				tinyxml2::XMLElement* XmlElemNodes = XmlDoc->NewElement("Nodes");
				for (auto& Node : AnimGraphNodes)
				{
					// <Node Name="NodeName" Type="NodeType" ...>
					tinyxml2::XMLElement* XmlElemNode = XmlDoc->NewElement("Node");
					XmlElemNode->SetAttribute("Name", Node->NodeName.c_str());
					XmlElemNode->SetAttribute("Type", Node->NodeTypeName.c_str());

					// Write custom attributes
					for (const auto& Iter : Node->Attributes.Map)
					{
						XmlElemNode->SetAttribute(Iter.first.c_str(), Iter.second.c_str());
					}

					// Write child entries
					for (const auto& EntryIter : Node->Attributes.ChildEntries)
					{
						// <ChildEntry AttributeName="AttributeValue" ...>
						tinyxml2::XMLElement* XmlElemChildEntry = XmlDoc->NewElement(EntryIter.EntryName.c_str());
						for (const auto& Iter : EntryIter.Map)
						{
							XmlElemChildEntry->SetAttribute(Iter.first.c_str(), Iter.second.c_str());
						}
						XmlElemNode->InsertEndChild(XmlElemChildEntry);
						// </ChildEntry>
					}

					// Write input node entires
					for (int i = 0; i < (int)Node->Inputs.size(); i++)
					{
						// <Input NodeName="InputNodeName" Index="0">
						tinyxml2::XMLElement* XmlElemNodeInput = XmlDoc->NewElement("Input");
						XmlElemNodeInput->SetAttribute("NodeName", Node->Inputs[i].c_str());
						XmlElemNodeInput->SetAttribute("Index", std::to_string(i).c_str());
						XmlElemNode->InsertEndChild(XmlElemNodeInput);
					}

					XmlElemNodes->InsertEndChild(XmlElemNode);
					// </Node>
				}

				XmlElemAnimGraph->InsertEndChild(XmlElemNodes);
			}
			// </Nodes>

			// <RootNode Name="RootNodeName">
			{
				// Root node of this anim graph
				tinyxml2::XMLElement* XmlElemRootNode = XmlDoc->NewElement("RootNode");
				if (RootGraphNode)
				{
					XmlElemRootNode->SetAttribute("Name", RootGraphNode->NodeName.c_str());
				}
				XmlElemAnimGraph->InsertEndChild(XmlElemRootNode);
			}
			// </RootNode>
		}
		XmlDoc->InsertEndChild(XmlElemAnimGraph);
		// </AnimGraph>
	}
	return XmlDoc->SaveFile(GetFileSystemPath().c_str()) == tinyxml2::XML_NO_ERROR;
}

std::unique_ptr<RAnimNode_Base> RAnimGraph::CreateAnimNode(const std::string& NodeName, const std::string& TypeName, const AnimNodeAttributeMap& Attributes)
{
	// Find a factory method for the type name
	auto Iter = AnimNodeFactoryMethods.find(TypeName);
	if (Iter != AnimNodeFactoryMethods.end())
	{
		return (*Iter->second.Method)(NodeName, Attributes);
	}

	RLogWarning("RAnimGraph::CreateAnimNode - Cannot create node for type '%s'. Type is not registered.\n", TypeName.c_str());
	return nullptr;
}

std::unique_ptr<RAnimNode_Base> RAnimGraph::CreateAnimNode(const RAnimGraphNode& AnimGraphNode)
{
	return CreateAnimNode(AnimGraphNode.NodeName, AnimGraphNode.NodeTypeName, AnimGraphNode.Attributes);
}

int RAnimGraph::GetNumInputPosesOfNodeType(const std::string& TypeName)
{
	auto Iter = AnimNodeFactoryMethods.find(TypeName);
	if (Iter != AnimNodeFactoryMethods.end())
	{
		return Iter->second.NumInputPoses;
	}

	return -1;
}

std::string RAnimGraph::MakeUniqueNodeName(const std::string& BaseName) const
{
	int Index = 0;

	while (1)
	{
		std::string OutName = BaseName + std::string("_") + std::to_string(Index);
		bool bIsDuplicatedName = false;

		for (auto& GraphNode : AnimGraphNodes)
		{
			if (OutName == GraphNode->NodeName)
			{
				bIsDuplicatedName = true;
				break;
			}
		}

		if (!bIsDuplicatedName)
		{
			return OutName;
		}

		Index++;
	}
}

RAnimGraphNode* RAnimGraph::FindGraphNodeByName(const std::string& Name) const
{
	for (auto& Node : AnimGraphNodes)
	{
		if (StringUtils::EqualsIgnoreCase(Node->NodeName, Name))
		{
			return Node.get();
		}
	}

	return nullptr;
}

std::vector<RAnimGraphNode*> RAnimGraph::CollectAllNodes() const
{
	std::vector<RAnimGraphNode*> Results;

	if (RootGraphNode)
	{
		Results.push_back(RootGraphNode);

		std::vector<std::string> NodeNames = RootGraphNode->Inputs;
		while (NodeNames.size())
		{
			if (RAnimGraphNode* ChildNode = FindGraphNodeByName(NodeNames[0]))
			{
				if (!StdContains(Results, ChildNode))
				{
					Results.push_back(ChildNode);
					NodeNames.insert(NodeNames.end(), ChildNode->Inputs.begin(), ChildNode->Inputs.end());
				}

				// Remove the head element
				NodeNames.erase(NodeNames.begin());
			}
		}
	}

	return Results;
}

RAnimGraphInstance::RAnimGraphInstance()
	: RootNode(nullptr)
{

}

void RAnimGraphInstance::Update(float DeltaTime)
{
	if (RootNode)
	{
		RootNode->UpdateNode(DeltaTime);
	}
}

void RAnimGraphInstance::EvaluatePose(RAnimPoseData& PoseData)
{
	if (RootNode)
	{
		RootNode->EvaluatePose(PoseData);
	}
}

RAnimNode_Base* RAnimGraphInstance::FindNodeByName(const std::string& NodeName) const
{
	for (auto& Node : Nodes)
	{
		if (StringUtils::EqualsIgnoreCase(Node->GetName(), NodeName))
		{
			return Node.get();
		}
	}

	return nullptr;
}
