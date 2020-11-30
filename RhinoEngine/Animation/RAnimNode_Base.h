//=============================================================================
// RAnimNode_Base.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

class RMesh;
class RAnimNode_Base;

typedef std::map<std::string, std::string> AttributeMap;

// A collection of animation node attributes consisting of attribute keys and values.
struct AnimNodeAttributeMap
{
	struct ChildEntry
	{
		std::string EntryName;
		AttributeMap Map;

		FORCEINLINE std::string& operator[](const std::string& Key)
		{
			return Map[Key];
		}
	};

	AttributeMap Map;
	std::vector<ChildEntry> ChildEntries;

	FORCEINLINE std::string& operator[](const std::string& Key)
	{
		return Map[Key];
	}

	// Queries a key and returns its value as string.
	// If the key doesn't exist, OutValue will remain unchanged.
	// Returns true if key exists.
	static bool Query(const AttributeMap& Map, const std::string& KeyName, std::string& OutValue);

	// Queries a key and returns its value as bool.
	// If the key doesn't exist, OutBool will remain unchanged.
	// Returns true if key exists.
	static bool QueryBool(const AttributeMap& Map, const std::string& KeyName, bool& OutBool);

	static bool QueryFloat(const AttributeMap& Map, const std::string& KeyName, float& OutFloat);
};

#define DECLARE_ANIM_NODE_FACTORY_METHOD(ClassName) \
	static const std::string& GetTypeName() { \
		static const std::string StaticTypeName(#ClassName + 1); /* Remove the first letter 'R' from class name */ \
		return StaticTypeName; \
	} \
	static std::unique_ptr<RAnimNode_Base> FactoryCreate(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes) { return std::make_unique<ClassName>(InNodeName, Attributes); }
	

// Animation pose data contains all the bone transformations of a skinned mesh at one moment in time.
struct RAnimPoseData
{
	RAnimPoseData(const RMesh& InSkinnedMesh);

	RAnimPoseData(const RAnimPoseData& Other)
		: SkinnedMesh(Other.SkinnedMesh)
		, BoneMatrices(Other.BoneMatrices)
	{
	}

	RAnimPoseData(RAnimPoseData&& Other)
		: SkinnedMesh(std::move(Other.SkinnedMesh))
		, BoneMatrices(std::move(Other.BoneMatrices))
	{
	}

	RAnimPoseData& operator=(const RAnimPoseData& Other)
	{
		SkinnedMesh = Other.SkinnedMesh;
		BoneMatrices = Other.BoneMatrices;
		return *this;
	}

	RAnimPoseData& operator=(RAnimPoseData&& Other)
	{
		SkinnedMesh = std::move(Other.SkinnedMesh);
		BoneMatrices = std::move(Other.BoneMatrices);
		return *this;
	}

	// Convert all metrics into world space and output to the metrics array
	void CopyFinalPose(const RMatrix4& ObjectToWorld, RMatrix4* OutMetrics);

	// Blend together two poses
	static RAnimPoseData BlendTwoPoses(const RAnimPoseData& Pose1, const RAnimPoseData& Pose2, float BlendFactor);

	// The skinned mesh used in pose evaluation
	const RMesh* SkinnedMesh;

	// Transforms for each bone in object space
	std::vector<RMatrix4> BoneMatrices;
};


template<typename T>
struct AnimVariable
{
	AnimVariable(T InVal)
		: Val(InVal)
		, SourcePtr(nullptr)
	{}

	operator const T() const	{ return Val; }
	operator T()				{ return Val; }

	// Update the variable by the pointer it's assigned to
	void UpdateVal()			{ if (SourcePtr) { Val = *SourcePtr; } }

	T Val;
	T* SourcePtr;
};


// The base class for all animation nodes
class RAnimNode_Base
{
public:
	RAnimNode_Base();
	RAnimNode_Base(const std::string& InNodeName, const AnimNodeAttributeMap& Attributes);
	virtual ~RAnimNode_Base();

	virtual void UpdateNode(float DeltaTime);

	// Evaluate pose for this node and any ancestor nodes
	virtual void EvaluatePose(RAnimPoseData& PoseData);

	virtual bool BindAnimVariable(const std::string& VariableName, float* ValPtr);

private:
	std::string NodeName;
};

