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

std::map<const std::string, AnimNodeFactoryMethod> RAnimGraph::AnimNodeFactoryMethods;


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
	if (RootGraphNode)
	{
		std::unique_ptr<RAnimNode_Base> RootNode = CreateAnimNode(*RootGraphNode);
		RawAnimGraphInstance->Nodes.push_back(RootNode.get());
		RawAnimGraphInstance->RootNode = std::move(RootNode);
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
			BaseNode->Inputs.resize(InputIndex + 1, nullptr);
		}
		BaseNode->Inputs[InputIndex] = NewNode.get();
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
}

void RAnimGraph::RegisterAnimNodeType(const std::string& TypeName, AnimNodeFactoryMethod FactoryMethod)
{
	AnimNodeFactoryMethods[TypeName] = FactoryMethod;
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

						// Read child entries
						const tinyxml2::XMLElement* XmlElemChildEntry = XmlElemNode->FirstChildElement();
						while (XmlElemChildEntry)
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

							XmlElemChildEntry = XmlElemChildEntry->NextSiblingElement();
						}

						auto NewNode = std::make_unique<RAnimGraphNode>(NodeName, NodeTypeName);
						NewNode->Attributes = std::move(AttributeMap);

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
		tinyxml2::XMLElement* XmlElemAnimGraph = XmlDoc->NewElement("AnimGraph");
		{
			// All nodes of this anim graph
			tinyxml2::XMLElement* XmlElemNodes = XmlDoc->NewElement("Nodes");

			// Root node of this anim graph
			tinyxml2::XMLElement* XmlElemRootNode = XmlDoc->NewElement("RootNode");

			for (auto& Node : AnimGraphNodes)
			{
				tinyxml2::XMLElement* XmlElemNode = XmlDoc->NewElement("Node");

				// Write name of graph node
				XmlElemNode->SetAttribute("Name", Node->NodeName.c_str());

				// Write type of graph node
				XmlElemNode->SetAttribute("Type", Node->NodeTypeName.c_str());

				// Write custom attributes
				for (const auto& Iter : Node->Attributes.Map)
				{
					XmlElemNode->SetAttribute(Iter.first.c_str(), Iter.second.c_str());
				}

				// Write child entries
				for (const auto& EntryIter : Node->Attributes.ChildEntries)
				{
					// Child entry name
					tinyxml2::XMLElement* XmlElemChildEntry = XmlDoc->NewElement(EntryIter.EntryName.c_str());

					// Child attributes
					for (const auto& Iter : EntryIter.Map)
					{
						XmlElemChildEntry->SetAttribute(Iter.first.c_str(), Iter.second.c_str());
					}

					XmlElemNode->InsertEndChild(XmlElemChildEntry);
				}

				XmlElemNodes->InsertEndChild(XmlElemNode);

				if (Node.get() == RootGraphNode)
				{
					XmlElemRootNode->SetAttribute("Name", Node->NodeName.c_str());
				}
			}

			XmlElemAnimGraph->InsertEndChild(XmlElemNodes);
			XmlElemAnimGraph->InsertEndChild(XmlElemRootNode);
		}
		XmlDoc->InsertEndChild(XmlElemAnimGraph);
	}
	return XmlDoc->SaveFile(GetFileSystemPath().c_str()) == tinyxml2::XML_NO_ERROR;
}

std::unique_ptr<RAnimNode_Base> RAnimGraph::CreateAnimNode(const std::string& NodeName, const std::string& TypeName, const AnimNodeAttributeMap& Attributes)
{
	// Find a factory method for the type name
	auto Iter = AnimNodeFactoryMethods.find(TypeName);
	if (Iter != AnimNodeFactoryMethods.end())
	{
		return (*Iter->second)(NodeName, Attributes);
	}

	RLogWarning("RAnimGraph::CreateAnimNode - Cannot create node for type '%s'. Type is not registered.\n", TypeName.c_str());
	return nullptr;
}

std::unique_ptr<RAnimNode_Base> RAnimGraph::CreateAnimNode(const RAnimGraphNode& AnimGraphNode)
{
	return CreateAnimNode(AnimGraphNode.NodeName, AnimGraphNode.NodeTypeName, AnimGraphNode.Attributes);
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

RAnimGraphInstance::RAnimGraphInstance()
	: RootNode(nullptr)
{

}

void RAnimGraphInstance::Update(float DeltaTime)
{
	if (RAnimNode_Base* RawRootNode = RootNode.get())
	{
		RawRootNode->UpdateNode(DeltaTime);
	}
}

void RAnimGraphInstance::EvaluatePose(RAnimPoseData& PoseData)
{
	if (RAnimNode_Base* RawRootNode = RootNode.get())
	{
		RawRootNode->EvaluatePose(PoseData);
	}
}

void RAnimGraphInstance::BindAnimVariable(const std::string& VariableName, float* ValPtr)
{
	// TODO: Support format like "NodeName:VariableName"
	//		 Search anim node by name and locate a unique variable in the graph
	if (RAnimNode_Base* RawRootNode = RootNode.get())
	{
		RawRootNode->BindAnimVariable(VariableName, ValPtr);
	}
}
