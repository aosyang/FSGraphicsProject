//=============================================================================
// RFbxMeshLoader.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "RFbxMeshLoader.h"

#include "RenderSystem/RAnimation.h"
#include "RenderSystem/RMaterial.h"
#include "RenderSystem/RMeshElement.h"

#include "Core/RLog.h"
#include "RResourceManager.h"
#include "RenderSystem/RMesh.h"

#include <fbxsdk.h>

/// When enabled, meshes and animations will be imported in left-handed coordinate
/// TODO: Handle fbx coordinate system
#define CONVERT_TO_LEFT_HANDED_MESH 1

/// Output matrices to log during importing for debugging
#define DEBUG_LOG_MATRICES 0

namespace
{
	/// Load animation of the scene
	RAnimation* LoadFbxSceneAnimation(FbxScene* Scene);

	/// Load materials from fbx node
	void LoadFbxMaterials(FbxNode* SceneNode, std::vector<RMaterial*>& OutMaterials);

	/// A helper function to convert fbx matrix to RMatrix4
	void MatrixTransfer(RMatrix4& Dest, const FbxAMatrix& Src);

	/// Get the matrix of fbx scene node
	RMatrix4 GetFbxNodeTransform(FbxNode* SceneNode);

	/// Get readable string format for matrix
	std::string GetDisplayStringForMatrix(const RMatrix4& Matrix);
}


bool RFbxMeshLoader::LoadDataForMeshResource(RMesh* MeshResource, const char* FileName)
{
	std::vector<std::unique_ptr<RMeshElement>> meshElements;
	std::vector<RMaterial*> materials;

	RLog("Loading mesh [%s]...\n", FileName);

	// Create the FBX SDK manager
	FbxManager* lFbxSdkManager = FbxManager::Create();

	// Create an IOSettings object.
	FbxIOSettings * ios = FbxIOSettings::Create(lFbxSdkManager, IOSROOT);
	lFbxSdkManager->SetIOSettings(ios);

	// ... Configure the FbxIOSettings object ...

	// Import options determine what kind of data is to be imported.
	// True is the default, but here we’ll set some to true explicitly, and others to false.
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL, true);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE, true);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK, false);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE, false);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO, false);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION, true);
	(*(lFbxSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(lFbxSdkManager, "");

	// Declare the path and filename of the file containing the scene.
	// In this case, we are assuming the file is in the same directory as the executable.

	// Initialize the importer.
	bool lImportStatus = lImporter->Initialize(FileName, -1, lFbxSdkManager->GetIOSettings());

	if (!lImportStatus) {
		RLogError("Call to FbxImporter::Initialize() failed.\n");
		RLogError("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());

		lFbxSdkManager->Destroy();
		return false;
	}

	// File format version numbers to be populated.
	int lFileMajor, lFileMinor, lFileRevision;

	// Populate the FBX file format version numbers with the import file.
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	// Create a new scene so it can be populated by the imported file.
	FbxScene* lFbxScene = FbxScene::Create(lFbxSdkManager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(lFbxScene);

	// The file has been imported; we can get rid of the importer.
	lImporter->Destroy();

	// Convert mesh, NURBS and patch into triangle mesh
	FbxGeometryConverter lGeomConverter(lFbxSdkManager);

	lGeomConverter.Triangulate(lFbxScene, true, true);
	//bool result = lGeomConverter.SplitMeshesPerMaterial(lFbxScene, true);

	FbxGlobalSettings& GlobalSettings = lFbxScene->GetGlobalSettings();
	FbxAxisSystem AxisSystem = GlobalSettings.GetAxisSystem();
	FbxSystemUnit SystemUnit = GlobalSettings.GetSystemUnit();

	// Load skinning nodes
	std::vector<FbxNode*> fbxBoneNodes;
	std::vector<std::string> meshBoneIdToName;
	int NumFbxNodes = lFbxScene->GetNodeCount();

	// Load bone information into an array
	for (int IdxNode = 0; IdxNode < NumFbxNodes; IdxNode++)
	{
		FbxNode* SkeletonNode = lFbxScene->GetNode(IdxNode);
		if (SkeletonNode)
		{
			FbxNodeAttribute* NodeAttribute = SkeletonNode->GetNodeAttribute();
			if (NodeAttribute)
			{
				if (NodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
				{
					fbxBoneNodes.push_back(SkeletonNode);
					meshBoneIdToName.push_back(SkeletonNode->GetName());

					RLogVerbose("  FBX bone node: %s\n", SkeletonNode->GetName());
				}
			}
		}
	}

	// Load scene animation
	RAnimation* animation = LoadFbxSceneAnimation(lFbxScene);
	if (animation)
	{
		animation->SetName(MeshResource->GetAssetPath());
		const std::string& SkeletalMeshName = MeshResource->GetMetaData()["SkeletalMesh"];
		if (SkeletalMeshName != "")
		{
			RMesh* SkeletalMesh = RResourceManager::Instance().LoadResource<RMesh>(SkeletalMeshName, EResourceLoadMode::Immediate);
			if (SkeletalMesh)
			{
				SkeletalMesh->CacheAnimation(animation);
			}
		}
	}

	std::vector<RMatrix4> boneInitInvPose;

	// Load meshes
	for (int IdxNode = 0; IdxNode < NumFbxNodes; IdxNode++)
	{
		FbxNode* SceneNode = lFbxScene->GetNode(IdxNode);
		const char* NodeName = SceneNode->GetName();
		RLogVerbose("  FBX node [%d/%d]: %s\n", IdxNode + 1, NumFbxNodes, NodeName);

		FbxMesh* MeshNode = SceneNode->GetMesh();

		if (!MeshNode)
		{
			continue;
		}

		//mesh->SplitPoints();
		RLogVerbose("    Found mesh element! [%s]\n", NodeName);

		FbxVector4* controlPointArray;
		std::vector<RVertexType::MeshLoader> vertData;
		int VertexComponentMask = 0;

		controlPointArray = MeshNode->GetControlPoints();
		int controlPointCount = MeshNode->GetControlPointsCount();

		vertData.resize(controlPointCount);

		RMatrix4 NodeTransform = GetFbxNodeTransform(SceneNode);

		// Fill vertex data
		for (int i = 0; i < controlPointCount; i++)
		{
			RVec3 Position(
				(float)controlPointArray[i][0],
				(float)controlPointArray[i][1],
				(float)controlPointArray[i][2]);

#if CONVERT_TO_LEFT_HANDED_MESH == 1
			Position.SetZ(-Position.Z());
#endif

			// Bake node transform to vertices
			Position = NodeTransform.Transform(Position);

			vertData[i].pos.x = Position.X();
			vertData[i].pos.y = Position.Y();
			vertData[i].pos.z = Position.Z();

			memset(vertData[i].boneId, -1, sizeof(int) * 4);
			memset(&vertData[i].weight, 0, sizeof(float) * 4);

			VertexComponentMask |= VCM_Pos;
		}

		// Fill normal data
		FbxGeometryElementNormal* normalArray = MeshNode->GetElementNormal();
		bool hasPerPolygonVertexNormal = false;

		if (normalArray)
		{
			hasPerPolygonVertexNormal = (normalArray->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

			switch (normalArray->GetMappingMode())
			{
			case FbxGeometryElement::eByControlPoint:
				switch (normalArray->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					for (int i = 0; i < controlPointCount; i++)
					{
						FbxVector4 normal = normalArray->GetDirectArray().GetAt(i);

						vertData[i].normal.x = (float)normal[0];
						vertData[i].normal.y = (float)normal[1];
						vertData[i].normal.z = (float)normal[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
						vertData[i].normal.z = -vertData[i].normal.z;
#endif

						VertexComponentMask |= VCM_Normal;
					}
					break;

				case FbxGeometryElement::eIndexToDirect:
					for (int i = 0; i < controlPointCount; i++)
					{
						int index = normalArray->GetIndexArray().GetAt(i);
						FbxVector4 normal = normalArray->GetDirectArray().GetAt(index);

						vertData[i].normal.x = (float)normal[0];
						vertData[i].normal.y = (float)normal[1];
						vertData[i].normal.z = (float)normal[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
						vertData[i].normal.z = -vertData[i].normal.z;
#endif

						VertexComponentMask |= VCM_Normal;
					}
					break;
				}
				break;
			}
		}


		FbxGeometryElementUV* uvArray[2] =
		{
			MeshNode->GetElementUV(0),
			MeshNode->GetElementUV(1),
		};

		bool hasPerPolygonVertexUV[2];

		for (int uvLayer = 0; uvLayer < 2; uvLayer++)
		{
			hasPerPolygonVertexUV[uvLayer] = uvArray[uvLayer] ? (uvArray[uvLayer]->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) : false;

			if (uvArray[uvLayer] && !hasPerPolygonVertexUV)
			{
				switch (uvArray[uvLayer]->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					for (int i = 0; i < controlPointCount; i++)
					{
						RVertexType::Vec2Data& vertUV = uvLayer == 0 ? vertData[i].uv0 : vertData[i].uv1;

						FbxVector2 uv = uvArray[i]->GetDirectArray().GetAt(i);

						vertUV.x = (float)uv[0];
						vertUV.y = 1.0f - (float)uv[1];

						if (uvLayer == 0)
							VertexComponentMask |= VCM_UV0;
						else
							VertexComponentMask |= VCM_UV1;
					}
					break;

				case FbxGeometryElement::eIndexToDirect:
					for (int i = 0; i < controlPointCount; i++)
					{
						RVertexType::Vec2Data& vertUV = uvLayer == 0 ? vertData[i].uv0 : vertData[i].uv1;

						int index = uvArray[uvLayer]->GetIndexArray().GetAt(i);
						FbxVector2 uv = uvArray[uvLayer]->GetDirectArray().GetAt(index);

						vertUV.x = (float)uv[0];
						vertUV.y = 1.0f - (float)uv[1];

						if (uvLayer == 0)
							VertexComponentMask |= VCM_UV0;
						else
							VertexComponentMask |= VCM_UV1;
					}
					break;
				}
			}
		}

		// Tangent
		FbxGeometryElementTangent* tangentArray = MeshNode->GetElementTangent();

		// If the mesh doesn't have tangents, try generating a set of them
		if (!tangentArray)
		{
			// Normal and UV0 are required to generate tangents
			if (normalArray && uvArray[0])
			{
				if (MeshNode->GenerateTangentsData(0))
				{
					tangentArray = MeshNode->GetElementTangent();
				}
			}
		}

		bool hasPerPolygonVertexTangent = false;
		if (tangentArray)
		{
			hasPerPolygonVertexTangent = (tangentArray->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

			if (!hasPerPolygonVertexTangent)
			{
				switch (tangentArray->GetReferenceMode())
				{
				case FbxGeometryElement::eDirect:
					for (int i = 0; i < controlPointCount; i++)
					{
						FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(i);

						vertData[i].tangent.x = (float)tangent[0];
						vertData[i].tangent.y = (float)tangent[1];
						vertData[i].tangent.z = (float)tangent[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
						vertData[i].tangent.z = -vertData[i].tangent.z;
#endif

						VertexComponentMask |= VCM_Tangent;
					}
					break;

				case FbxGeometryElement::eIndexToDirect:
					for (int i = 0; i < controlPointCount; i++)
					{
						int index = tangentArray->GetIndexArray().GetAt(i);
						FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(index);

						vertData[i].tangent.x = (float)tangent[0];
						vertData[i].tangent.y = (float)tangent[1];
						vertData[i].tangent.z = (float)tangent[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
						vertData[i].tangent.z = -vertData[i].tangent.z;
#endif

						VertexComponentMask |= VCM_Tangent;
					}
					break;
				}
			}
		}


		bool hasDeformer = (MeshNode->GetDeformer(0, FbxDeformer::eSkin) != NULL);
		if (hasDeformer)
		{
			int deformerCount = MeshNode->GetDeformerCount();

			for (int idxSkinDeformer = 0; idxSkinDeformer < deformerCount; idxSkinDeformer++)
			{
				// A deformer on a mesh is a skinning controller that keeps all cluster (bone) information
				FbxSkin* skinDeformer = (FbxSkin*)MeshNode->GetDeformer(idxSkinDeformer, FbxDeformer::eSkin);

				int clusterCount = skinDeformer->GetClusterCount();
				for (int idxCluster = 0; idxCluster < clusterCount; idxCluster++)
				{
					// A cluster is structure contains bone node, affected points and weight of affection
					// Binding pose matrix can also be retrieved from cluster
					FbxCluster* cluster = skinDeformer->GetCluster(idxCluster);
					FbxNode* LinkNode = cluster->GetLink();

					if (!LinkNode)
						continue;

					int boneId = (int)(std::find(fbxBoneNodes.begin(), fbxBoneNodes.end(), LinkNode) - fbxBoneNodes.begin());
					assert(boneId < MAX_BONE_COUNT);

					// Store inversed initial transform for each bone to apply skinning with correct binding pose
					FbxAMatrix clusterInitTransform;
					cluster->GetTransformLinkMatrix(clusterInitTransform);

#if DEBUG_LOG_MATRICES == 1
					{
						RMatrix4 OriginalTransform;
						MatrixTransfer(OriginalTransform, clusterInitTransform);
						RLogVerbose("Cluster link \'%s\', initial transform:\n%s", LinkNode->GetName(), GetDisplayStringForMatrix(OriginalTransform).c_str());
					}
#endif	// DEBUG_LOG_MATRICES == 1

#if CONVERT_TO_LEFT_HANDED_MESH == 1
					FbxVector4 rotation = clusterInitTransform.GetR();
					clusterInitTransform[3][2] = -clusterInitTransform[3][2];
					rotation.Set(-rotation[0], -rotation[1], rotation[2]);
					clusterInitTransform.SetR(rotation);
#endif
					clusterInitTransform = clusterInitTransform.Inverse();

					if (boneInitInvPose.size() == 0 && fbxBoneNodes.size())
						boneInitInvPose.resize(fbxBoneNodes.size());

					if (boneInitInvPose.size() != 0)
						MatrixTransfer(boneInitInvPose[boneId], clusterInitTransform);

					int cpIndicesCount = cluster->GetControlPointIndicesCount();
					for (int idxCpIndex = 0; idxCpIndex < cpIndicesCount; idxCpIndex++)
					{
						// Note: A control point is a point affected by this cluster (bone)

						int index = cluster->GetControlPointIndices()[idxCpIndex];
						float weight = (float)cluster->GetControlPointWeights()[idxCpIndex];
						bool bBoneDataUpdated = false;
						float MinWeight = 0.0f;
						int MinIndex = -1;

						// Store bone id and weight in an empty slot of vertex skinning attributes
						for (int i = 0; i < 4; i++)
						{
							if (vertData[index].boneId[i] == -1)
							{
								vertData[index].boneId[i] = boneId;
								vertData[index].weight[i] = weight;

								VertexComponentMask |= VCM_BoneId;
								VertexComponentMask |= VCM_BoneWeights;

								bBoneDataUpdated = true;
								break;
							}

							if (MinIndex == -1 || vertData[index].weight[i] < MinWeight)
							{
								MinIndex = i;
								MinWeight = vertData[index].weight[i];
							}
						}

						// If a vertex is affected by more than 4 bones, replace a minimal weighted bone with the new one
						if (!bBoneDataUpdated)
						{
							if (MinIndex != -1 && weight > MinWeight)
							{
								vertData[index].boneId[MinIndex] = boneId;
								vertData[index].weight[MinIndex] = weight;
							}
						}
					}

				}
			}
		}

		// Set bone id in unused slot to 0 so shader won't mess up
		for (UINT32 n = 0; n < vertData.size(); n++)
		{
			float TotalBoneWeight = vertData[n].weight[0] + vertData[n].weight[1] + vertData[n].weight[2] + vertData[n].weight[3];
			float InvTotalWeight = FLT_EQUAL_ZERO(TotalBoneWeight) ? 1.0f : 1.0f / TotalBoneWeight;

			for (int i = 0; i < 4; i++)
			{
				if (vertData[n].boneId[i] == -1)
				{
					vertData[n].boneId[i] = 0;
					vertData[n].weight[i] = 0.0f;
				}

				// Renormalize all weights so they add up to 1
				vertData[n].weight[i] *= InvTotalWeight;
			}
		}

		FbxArray<int> PolygonMaterialIds;

		int NumLayers = MeshNode->GetLayerCount();
		for (int IdxLayer = 0; IdxLayer < NumLayers; IdxLayer++)
		{
			FbxLayer* Layer = MeshNode->GetLayer(IdxLayer);
			if (Layer)
			{
				FbxLayerElementMaterial* Materials = Layer->GetMaterials();
				if (Materials)
				{
					// Note: If index array is stored in a non-reference variable by accident,
					//       it will get copied and cause a crash when releasing fbx scene. (Please, FBX SDK!!)
					FbxLayerElementArrayTemplate<int>& IndexArray = Materials->GetIndexArray();
					int NumIndices = IndexArray.GetCount();

					IndexArray.CopyTo(PolygonMaterialIds);

					// For now, we don't handle multiple layers
					break;
				}
			}
		}

		FbxArray<int> UniqueArray;
		int NumPolygonMaterials = -1;

		// Get number of polygon materials
		{
			for (int i = 0; i < PolygonMaterialIds.GetCount(); i++)
			{
				UniqueArray.AddUnique(PolygonMaterialIds[i]);
			}

			NumPolygonMaterials = UniqueArray.GetCount();
		}

		if (NumPolygonMaterials <= 0)
		{
			NumPolygonMaterials = 1;
		}

		std::vector<std::vector<UINT>> SubmeshIndexArray;
		SubmeshIndexArray.resize(NumPolygonMaterials);

		std::vector<RVertexType::MeshLoader> flatVertData;

		// Fill triangle data
		int polyCount = MeshNode->GetPolygonCount();

		for (int idxPoly = 0; idxPoly < polyCount; idxPoly++)
		{
			// Loop through index buffer
			int vertCountPerPoly = MeshNode->GetPolygonSize(idxPoly);

			assert(vertCountPerPoly == 3);

			// Note: Assume mesh has been triangulated
			int triangle[3];

			for (int idxVert = 0; idxVert < vertCountPerPoly; idxVert++)
			{
				//triangle[idxVert] = mesh->GetPolygonVertex(idxPoly, idxVert);
				triangle[idxVert] = idxPoly * 3 + idxVert;
				int iv = MeshNode->GetPolygonVertex(idxPoly, idxVert);

				RVertexType::MeshLoader vertex = vertData[iv];

				if (hasPerPolygonVertexNormal)
				{
					FbxVector4 normal;
					MeshNode->GetPolygonVertexNormal(idxPoly, idxVert, normal);

					vertex.normal.x = (float)normal[0];
					vertex.normal.y = (float)normal[1];
					vertex.normal.z = (float)normal[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
					vertex.normal.z = -vertex.normal.z;
#endif

					VertexComponentMask |= VCM_Normal;
				}

				if (hasPerPolygonVertexTangent)
				{
					switch (tangentArray->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
					{
						FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(idxPoly * 3 + idxVert);

						vertex.tangent.x = (float)tangent[0];
						vertex.tangent.y = (float)tangent[1];
						vertex.tangent.z = (float)tangent[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
						vertex.tangent.z = -vertex.tangent.z;
#endif

						VertexComponentMask |= VCM_Tangent;
					}
					break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int index = tangentArray->GetIndexArray().GetAt(idxPoly * 3 + idxVert);
						FbxVector4 tangent = tangentArray->GetDirectArray().GetAt(index);

						vertex.tangent.x = (float)tangent[0];
						vertex.tangent.y = (float)tangent[1];
						vertex.tangent.z = (float)tangent[2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
						vertex.tangent.z = -vertex.tangent.z;
#endif

						VertexComponentMask |= VCM_Tangent;
					}
					break;
					}
				}

				for (int uvLayer = 0; uvLayer < 2; uvLayer++)
				{
					if (uvArray[uvLayer] && hasPerPolygonVertexUV[uvLayer])
					{
						int idxUV = idxPoly * 3 + idxVert;
						if (uvArray[uvLayer]->GetReferenceMode() != FbxGeometryElement::eDirect)
						{
							idxUV = uvArray[uvLayer]->GetIndexArray().GetAt(idxPoly * 3 + idxVert);
						}

						FbxVector2 uv = uvArray[uvLayer]->GetDirectArray().GetAt(idxUV);

						RVertexType::Vec2Data& vertUV = uvLayer == 0 ? vertex.uv0 : vertex.uv1;

						vertUV.x = (float)uv[0];
						vertUV.y = 1.0f - (float)uv[1];

						if (uvLayer == 0)
							VertexComponentMask |= VCM_UV0;
						else
							VertexComponentMask |= VCM_UV1;
					}
				}

				flatVertData.push_back(vertex);
			}

			int Index = 0;
			if (idxPoly < PolygonMaterialIds.Size())
			{
				int MaterialId = PolygonMaterialIds[idxPoly];
				Index = UniqueArray.Find(MaterialId);
				assert(Index != -1);
			}

			// Change triangle clockwise if necessary
#if CONVERT_TO_LEFT_HANDED_MESH == 1
			SubmeshIndexArray[Index].push_back(triangle[0]);
			SubmeshIndexArray[Index].push_back(triangle[2]);
			SubmeshIndexArray[Index].push_back(triangle[1]);
#else
			SubmeshIndexArray[Index].push_back(triangle[0]);
			SubmeshIndexArray[Index].push_back(triangle[1]);
			SubmeshIndexArray[Index].push_back(triangle[2]);
#endif
		}

		for (auto& IndexData : SubmeshIndexArray)
		{
			// Make copy of vertex array for optimization
			auto VertexData = flatVertData;

			int OrignialNumVerts = (int)VertexData.size();
			int OriginalNumIndices = (int)IndexData.size() / 3;

			// Optimize the mesh
			OptimizeMesh(IndexData, VertexData);

			int OptimizedNumVerts = (int)VertexData.size();
			int OptimizedNumIndices = (int)IndexData.size() / 3;

			// Hack: don't use uv1 on skinned mesh
			if (hasDeformer)
				VertexComponentMask &= ~VCM_UV1;

			if (VertexData.size() != 0 && IndexData.size() != 0)
			{
				std::unique_ptr<RMeshElement> meshElem = std::make_unique<RMeshElement>();

				meshElem->SetVertices(VertexData, VertexComponentMask);
				meshElem->SetTriangles(IndexData);
				meshElem->UpdateRenderBuffer();
				meshElem->SetName(SceneNode->GetName());

				UINT flag = 0;
				if (hasDeformer)
					flag |= MEF_Skinned;

				meshElem->SetFlag(flag);
				meshElements.push_back(std::move(meshElem));

				RLogVerbose("Mesh element loaded with %d vertices and %d triangles (unoptimized: vert %d, triangle %d).\n",
					OptimizedNumVerts, OptimizedNumIndices, OrignialNumVerts, OriginalNumIndices);
			}
			else
			{
				RLogWarning("Mesh loader: Unable to add mesh element with index count %d, vertex count %d.\n",
					(int)IndexData.size(), (int)VertexData.size());
			}
		}

		// Load materials from fbx node
		LoadFbxMaterials(SceneNode, materials);
	}

	lFbxScene->Destroy();
	lFbxSdkManager->Destroy();

	// If a material file .rmtl for the mesh exists, override materials from fbx
	std::string mtlFilename = RFileUtil::ReplaceExtension(FileName, "rmtl");
	std::vector<std::string> MaterialNames = RMaterial::LoadNameListFromXml(mtlFilename);
	if (MaterialNames.size() > 0)
	{
		if (MaterialNames.size() >= materials.size())
		{
			std::vector<RMaterial*> OverrideMaterials;
			for (int i = 0; i < (int)MaterialNames.size(); i++)
			{
				RMaterial* Material = RResourceManager::Instance().FindResource<RMaterial>(MaterialNames[i]);
				if (!Material)
				{
					// Request a referenced material for the mesh. If loading is happening in a loader thread, loading resource
					// with immediate mode will also happen there.
					Material = RResourceManager::Instance().LoadResource<RMaterial>(MaterialNames[i], EResourceLoadMode::Immediate);
				}

				if (!Material)
				{
					Material = RMaterial::GetDefault();
				}

				assert(Material);
				OverrideMaterials.push_back(Material);
			}

			materials = OverrideMaterials;
		}
		else
		{
			RLogWarning("Loading %d materials from .rmtl, expecting %d. Ignoring material overriding.\n",
						(int)MaterialNames.size(), (int)materials.size());
		}
	}

	if (meshElements.size() > 0)
	{
		MeshResource->SetMeshElements(std::move(meshElements));
		MeshResource->SetMaterials(materials);
		MeshResource->UpdateAabb();
	}

	MeshResource->SetAnimation(animation);
	MeshResource->SetBoneNameList(meshBoneIdToName);
	MeshResource->SetBoneInitInvMatrices(boneInitInvPose);

	return true;
}

bool RFbxMeshLoader::LoadDataForMeshResource(RMesh* MeshResource, const std::string& FileName)
{
	return LoadDataForMeshResource(MeshResource, FileName.c_str());
}

void RFbxMeshLoader::OptimizeMesh(std::vector<UINT>& IndexData, std::vector<RVertexType::MeshLoader>& VertexData) const
{
	// Optimize mesh
	RLogVerbose("Optimizing mesh...\n");

	std::map<RVertexType::MeshLoader, int> meshVertIndexTable;
	std::vector<RVertexType::MeshLoader> optimizedVertData;
	std::vector<UINT> optimizedIndexData;
	UINT Index = 0;

	for (UINT i = 0; i < IndexData.size(); i++)
	{
		RVertexType::MeshLoader& v = VertexData[IndexData[i]];

		// Search vertex in index table by running lexicographical comparison
		auto iterResult = meshVertIndexTable.find(v);
		if (iterResult == meshVertIndexTable.end())
		{
			meshVertIndexTable[v] = Index;
			optimizedVertData.push_back(v);
			optimizedIndexData.push_back(Index);
			Index++;
		}
		else
		{
			optimizedIndexData.push_back(iterResult->second);
		}
	}

	IndexData = optimizedIndexData;
	VertexData = optimizedVertData;
}

namespace
{
	RAnimation* LoadFbxSceneAnimation(FbxScene* Scene)
	{
		RAnimation* animation = nullptr;

		FbxArray<FbxString*> animStackNameArray;
		Scene->FillAnimStackNameArray(animStackNameArray);
		if (animStackNameArray.GetCount() > 0)
		{
			int NumFbxNodes = Scene->GetNodeCount();

			FbxAnimStack* animStack = Scene->FindMember<FbxAnimStack>(animStackNameArray[0]->Buffer());
			FbxTakeInfo* takeInfo = Scene->GetTakeInfo(*(animStackNameArray[0]));

			FbxArrayDelete(animStackNameArray);

			FbxTime::EMode				animTimeMode;
			FbxTime						TimePerFrame, animStartTime, animEndTime;
			float						animFrameRate;

			FbxGlobalSettings& GlobalSettings = Scene->GetGlobalSettings();

			TimePerFrame.SetTime(0, 0, 0, 1, 0, GlobalSettings.GetTimeMode());
			animTimeMode = GlobalSettings.GetTimeMode();
			animFrameRate = (float)TimePerFrame.GetFrameRate(animTimeMode);
			animStartTime = takeInfo->mLocalTimeSpan.GetStart();
			animEndTime = takeInfo->mLocalTimeSpan.GetStop();

			int totalFrameCount = (int)(animEndTime.GetFrameCount(animTimeMode) - animStartTime.GetFrameCount(animTimeMode)) + 1;
			animation = new RAnimation(
				NumFbxNodes,
				totalFrameCount,
				(float)animStartTime.GetFrameCountPrecise(animTimeMode),
				(float)animEndTime.GetFrameCountPrecise(animTimeMode),
				animFrameRate);

			std::map<std::string, int> nodeNameToId;

			for (int FbxSceneNodeIndex = 0; FbxSceneNodeIndex < NumFbxNodes; FbxSceneNodeIndex++)
			{
				FbxNode* SceneNode = Scene->GetNode(FbxSceneNodeIndex);

				// Note: We can't skip non-skeletal nodes as some animations rely on them for animations.
				//FbxNodeAttribute* NodeAttribute = SceneNode->GetNodeAttribute();
				//if (NodeAttribute && NodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
				{
					const char* BoneName = SceneNode->GetName();

					for (FbxTime CurrentFrameTime = animStartTime;
						 CurrentFrameTime <= animEndTime;
						 CurrentFrameTime += TimePerFrame)
					{
						RMatrix4 BoneTransform;

						// Evaluate bone transform in model space
						FbxAMatrix FbxBoneTransform = SceneNode->EvaluateGlobalTransform(CurrentFrameTime);

#if DEBUG_LOG_MATRICES == 1
						if (CurrentFrameTime == animStartTime)
						{
							RMatrix4 OriginalTransform;
							MatrixTransfer(OriginalTransform, FbxBoneTransform);
							RLogVerbose("Bone \'%s\', pose at first frame:\n%s", BoneName, GetDisplayStringForMatrix(OriginalTransform).c_str());
						}
#endif	// DEBUG_LOG_MATRICES == 1

#if CONVERT_TO_LEFT_HANDED_MESH == 1
						FbxVector4 FbxBoneRotation = FbxBoneTransform.GetR();
						FbxBoneTransform[3][2] = -FbxBoneTransform[3][2];
						FbxBoneRotation.Set(-FbxBoneRotation[0], -FbxBoneRotation[1], FbxBoneRotation[2]);
						FbxBoneTransform.SetR(FbxBoneRotation);
#endif
						MatrixTransfer(BoneTransform, FbxBoneTransform);

						// Precise frames number in fractions
						float NumFramesAtCurrentTime = (float)CurrentFrameTime.GetFrameCountPrecise(animTimeMode);
						float NumFramesAtStartTime = (float)animStartTime.GetFrameCountPrecise(animTimeMode);

						int FrameIndex = (int)(NumFramesAtCurrentTime - NumFramesAtStartTime);
						animation->AddNodePoseAtFrame(FbxSceneNodeIndex, FrameIndex, &BoneTransform);
						animation->SetNodeName(FbxSceneNodeIndex, BoneName);

						nodeNameToId[BoneName] = FbxSceneNodeIndex;
					}
				}
			}

			// Determine parent nodes
			for (int FbxSceneNodeIndex = 0; FbxSceneNodeIndex < NumFbxNodes; FbxSceneNodeIndex++)
			{
				FbxNode* SceneNode = Scene->GetNode(FbxSceneNodeIndex);
				FbxNode* ParentNode = SceneNode->GetParent();
				int ParentId = -1;

				if (ParentNode)
				{
					auto Iter = nodeNameToId.find(ParentNode->GetName());
					if (Iter != nodeNameToId.end())
					{
						ParentId = Iter->second;
					}
				}

				animation->SetNodeParentId(FbxSceneNodeIndex, ParentId);
			}
		}

		return animation;
	}

	enum FbxMaterialMap
	{
		DiffuseMap,
		NormalMap,
		SpecularMap,
	};

	void LoadFbxMaterials(FbxNode* SceneNode, std::vector<RMaterial*>& OutMaterials)
	{
		int NumFbxMaterials = SceneNode->GetSrcObjectCount<FbxSurfaceMaterial>();

		for (int IdxMaterial = 0; IdxMaterial < NumFbxMaterials; IdxMaterial++)
		{
#if 0		// Do not extract any material from fbx
			FbxSurfaceMaterial* material = SceneNode->GetSrcObject<FbxSurfaceMaterial>(IdxMaterial);
			std::string MaterialName = material->GetName();

			// Create a new material from fbx material
			RMaterial* meshMaterial = RResourceManager::Instance().CreateNewResource<RMaterial>(MaterialName);
			meshMaterial->SetAssetPath(MaterialName);
			int NextTextureSlotIdx = 0;

			const char* texType[] =
			{
				FbxSurfaceMaterial::sDiffuse,
				FbxSurfaceMaterial::sNormalMap,
				FbxSurfaceMaterial::sSpecular,
			};

			bool bHasMaps[3] = { false };

			for (int idxTexProp = 0; idxTexProp < 3; idxTexProp++)
			{
				FbxProperty prop = material->FindProperty(texType[idxTexProp]);
				int layeredTexCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
				std::string textureName;

				for (int idxLayeredTex = 0; idxLayeredTex < layeredTexCount; idxLayeredTex++)
				{
					FbxLayeredTexture* layeredTex = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(idxLayeredTex));
					int texCount = layeredTex->GetSrcObjectCount<FbxTexture>();

					for (int idxTex = 0; idxTex < texCount; idxTex++)
					{
						FbxFileTexture* texture = FbxCast<FbxFileTexture>(layeredTex->GetSrcObject<FbxTexture>(idxTex));
						textureName = texture->GetFileName();
					}
				}

				if (layeredTexCount == 0)
				{
					int texCount = prop.GetSrcObjectCount<FbxTexture>();

					for (int idxTex = 0; idxTex < texCount; idxTex++)
					{
						FbxFileTexture* texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxTexture>(idxTex));
						textureName = texture->GetFileName();
					}
				}

				if (textureName.length() != 0)
				{
					if (!RFileUtil::CheckIsRelativePath(textureName))
					{
						textureName = RFileUtil::GetFileNameInPath(textureName);
					}

					std::string ddsFilename = RFileUtil::ReplaceExtension(textureName, "dds");
					RTexture* texture = RResourceManager::Instance().FindResource<RTexture>(ddsFilename);

					if (!texture)
					{
						texture = RResourceManager::Instance().LoadResource<RTexture>(ddsFilename, EResourceLoadMode::Immediate);
					}

					meshMaterial->GetTextureSlots().push_back(RTextureSlotData(texture, NextTextureSlotIdx));
					NextTextureSlotIdx++;

					bHasMaps[idxTexProp] = true;
				}
			}

			RShader* Shader = nullptr;
			if (bHasMaps[DiffuseMap])
			{
				if (bHasMaps[NormalMap])
				{
					if (bHasMaps[SpecularMap])
					{
						Shader = GShaderManager.FindShaderByName("BumpSpecularLighting");
					}
					else
					{
						Shader = GShaderManager.FindShaderByName("BumpLighting");
					}
				}
				else
				{
					Shader = GShaderManager.FindShaderByName("Lighting");
				}
			}
			else
			{
				Shader = GShaderManager.GetDefaultShader();
			}

			assert(Shader != nullptr);
			meshMaterial->SetShader(Shader);

			OutMaterials.push_back(meshMaterial);
#endif
		}

		if (NumFbxMaterials == 0)
		{
			OutMaterials.push_back(RMaterial::GetDefault());
		}
	}

	void MatrixTransfer(RMatrix4& Dest, const FbxAMatrix& Src)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				Dest.m[y][x] = (float)Src.Get(y, x);
			}
		}
	}

	RMatrix4 GetFbxNodeTransform(FbxNode* SceneNode)
	{
		FbxDouble3 NodeTranslation = SceneNode->LclTranslation.Get();
		FbxDouble3 NodeRotation = SceneNode->LclRotation.Get();
		FbxDouble3 NodeScaling = SceneNode->LclScaling.Get();

		RVec3 Translation((float)NodeTranslation[0], (float)NodeTranslation[1], (float)NodeTranslation[2]);
		RQuat Rotation = RQuat::Euler(RMath::DegreeToRadian((float)NodeRotation[0]),
									  RMath::DegreeToRadian((float)NodeRotation[1]),
									  RMath::DegreeToRadian((float)NodeRotation[2]));
		RVec3 Scale((float)NodeScaling[0], (float)NodeScaling[1], (float)NodeScaling[2]);

		// TODO: Handle rotation in the future
		return RTransform(Translation, RQuat::IDENTITY, Scale).GetMatrix();
	}


	std::string GetDisplayStringForMatrix(const RMatrix4& Matrix)
	{
		std::ostringstream StringStream;
		for (int i = 0; i < 4; i++)
		{
			StringStream << "[" << std::fixed
						 << std::setw(10) << Matrix.m[i][0] << ", "
						 << std::setw(10) << Matrix.m[i][1] << ", "
						 << std::setw(10) << Matrix.m[i][2] << ", "
						 << std::setw(10) << Matrix.m[i][3] << "]" << std::endl;
		}
		return StringStream.str();
	}
}
