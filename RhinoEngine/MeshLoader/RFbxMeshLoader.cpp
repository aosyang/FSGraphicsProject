//=============================================================================
// RFbxMeshLoader.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RFbxMeshLoader.h"

#define CONVERT_TO_LEFT_HANDED_MESH 1

bool RFbxMeshLoader::LoadMeshIntoResource(RMesh* MeshResource, const char* FileName)
{
	vector<RMeshElement> meshElements;
	vector<RMaterial> materials;

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

	// Load skinning nodes
	vector<FbxNode*> fbxBoneNodes;
	vector<string> meshBoneIdToName;
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

					RLog("  FBX bone node: %s\n", SkeletonNode->GetName());
				}
			}
		}
	}

	// Load scene animation
	RAnimation* animation = LoadFbxSceneAnimation(lFbxScene);

	vector<RMatrix4> boneInitInvPose;

	// Load meshes
	for (int IdxNode = 0; IdxNode < NumFbxNodes; IdxNode++)
	{
		FbxNode* SceneNode = lFbxScene->GetNode(IdxNode);
		const char* NodeName = SceneNode->GetName();
		RLog("  FBX node [%d/%d]: %s\n", IdxNode + 1, NumFbxNodes, NodeName);

		FbxMesh* MeshNode = SceneNode->GetMesh();

		if (!MeshNode)
		{
			continue;
		}

		//mesh->SplitPoints();
		RLog("    Found mesh element! [%s]\n", NodeName);

		FbxVector4* controlPointArray;
		vector<RVertexType::MeshLoader> vertData;
		int VertexComponentMask = 0;

		controlPointArray = MeshNode->GetControlPoints();
		int controlPointCount = MeshNode->GetControlPointsCount();

		vertData.resize(controlPointCount);

		// Fill vertex data
		for (int i = 0; i < controlPointCount; i++)
		{
			vertData[i].pos.x = (float)controlPointArray[i][0];
			vertData[i].pos.y = (float)controlPointArray[i][1];
			vertData[i].pos.z = (float)controlPointArray[i][2];

#if CONVERT_TO_LEFT_HANDED_MESH == 1
			vertData[i].pos.z = -vertData[i].pos.z;
#endif

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

					if (!cluster->GetLink())
						continue;

					int boneId = (int)(std::find(fbxBoneNodes.begin(), fbxBoneNodes.end(), cluster->GetLink()) - fbxBoneNodes.begin());
					assert(boneId < MAX_BONE_COUNT);

					// Store inversed initial transform for each bone to apply skinning with correct binding pose
					FbxAMatrix clusterInitTransform;
					cluster->GetTransformLinkMatrix(clusterInitTransform);
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
						MatrixTransfer(boneInitInvPose.data() + boneId, &clusterInitTransform);

					int cpIndicesCount = cluster->GetControlPointIndicesCount();
					for (int idxCpIndex = 0; idxCpIndex < cpIndicesCount; idxCpIndex++)
					{
						// Note: A control point is a point affected by this cluster (bone)

						int index = cluster->GetControlPointIndices()[idxCpIndex];
						float weight = (float)cluster->GetControlPointWeights()[idxCpIndex];

						// Store bone id and weight in an empty slot of vertex skinning attributes
						for (int i = 0; i < 4; i++)
						{
							if (vertData[index].boneId[i] == -1)
							{
								vertData[index].boneId[i] = boneId;
								vertData[index].weight[i] = weight;

								VertexComponentMask |= VCM_BoneId;
								VertexComponentMask |= VCM_BoneWeights;

								break;
							}
						}
					}

				}
			}
		}

		// Set bone id in unused slot to 0 so shader won't mess up
		for (UINT32 n = 0; n < vertData.size(); n++)
		{
			for (int i = 0; i < 4; i++)
			{
				if (vertData[n].boneId[i] == -1)
					vertData[n].boneId[i] = 0;
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

		vector<vector<UINT>> SubmeshIndexArray;
		SubmeshIndexArray.resize(NumPolygonMaterials);

		vector<RVertexType::MeshLoader> flatVertData;

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
				RMeshElement meshElem;

				meshElem.SetVertices(VertexData, VertexComponentMask);
				meshElem.SetTriangles(IndexData);
				meshElem.UpdateRenderBuffer();
				meshElem.SetName(SceneNode->GetName());

				UINT flag = 0;
				if (hasDeformer)
					flag |= MEF_Skinned;

				meshElem.SetFlag(flag);
				meshElements.push_back(meshElem);

				RLog("Mesh element loaded with %d vertices and %d triangles (unoptimized: vert %d, triangle %d).\n",
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
	string mtlFilename = RFileUtil::ReplaceExtension(FileName, "rmtl");
	vector<RMaterial> OverrideMaterials;
	if (RMaterial::LoadFromXmlFile(mtlFilename, OverrideMaterials))
	{
		if (OverrideMaterials.size() >= materials.size())
		{
			materials = OverrideMaterials;
		}
		else
		{
			RLogWarning("Loading %d materials from .rmtl, expecting %d. Ignoring material overriding.\n",
				(int)OverrideMaterials.size(), (int)materials.size());
		}
	}

	MeshResource->SetMeshElements(meshElements.data(), (UINT)meshElements.size());
	MeshResource->SetMaterials(materials.data(), (UINT)materials.size());
	MeshResource->UpdateAabb();
	MeshResource->SetAnimation(animation);
	MeshResource->SetBoneNameList(meshBoneIdToName);
	MeshResource->SetBoneInitInvMatrices(boneInitInvPose);

	return true;
}

bool RFbxMeshLoader::LoadMeshIntoResource(RMesh* MeshResource, const std::string& FileName)
{
	return LoadMeshIntoResource(MeshResource, FileName.c_str());
}

RAnimation* RFbxMeshLoader::LoadFbxSceneAnimation(FbxScene* Scene) const
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

		TimePerFrame.SetTime(0, 0, 0, 1, 0, Scene->GetGlobalSettings().GetTimeMode());
		animTimeMode = Scene->GetGlobalSettings().GetTimeMode();
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

		map<string, int> nodeNameToId;

		for (int FbxSceneNodeIndex = 0; FbxSceneNodeIndex < NumFbxNodes; FbxSceneNodeIndex++)
		{
			FbxNode* node = Scene->GetNode(FbxSceneNodeIndex);

			for (FbxTime CurrentFrameTime = animStartTime;
				CurrentFrameTime <= animEndTime;
				CurrentFrameTime += TimePerFrame)
			{
				RMatrix4 BoneTransform;
				const char* BoneName = node->GetName();

				// Evaluate bone transform in model space
				FbxAMatrix FbxBoneTransform = node->EvaluateGlobalTransform(CurrentFrameTime);

#if CONVERT_TO_LEFT_HANDED_MESH == 1
				FbxVector4 FbxBoneRotation = FbxBoneTransform.GetR();
				FbxBoneTransform[3][2] = -FbxBoneTransform[3][2];
				FbxBoneRotation.Set(-FbxBoneRotation[0], -FbxBoneRotation[1], FbxBoneRotation[2]);
				FbxBoneTransform.SetR(FbxBoneRotation);
#endif
				MatrixTransfer(&BoneTransform, &FbxBoneTransform);

				// Precise frames number in fractions
				float NumFramesAtCurrentTime = (float)CurrentFrameTime.GetFrameCountPrecise(animTimeMode);
				float NumFramesAtStartTime = (float)animStartTime.GetFrameCountPrecise(animTimeMode);

				int FrameIndex = (int)(NumFramesAtCurrentTime - NumFramesAtStartTime);
				animation->AddNodePose(FbxSceneNodeIndex, FrameIndex, &BoneTransform);
				animation->AddNodeNameToId(BoneName, FbxSceneNodeIndex);

				nodeNameToId[BoneName] = FbxSceneNodeIndex;
			}
		}

		for (int FbxSceneNodeIndex = 0; FbxSceneNodeIndex < NumFbxNodes; FbxSceneNodeIndex++)
		{
			FbxNode* node = Scene->GetNode(FbxSceneNodeIndex);
			FbxNode* parent = node->GetParent();
			if (parent && nodeNameToId.find(parent->GetName()) != nodeNameToId.end())
			{
				animation->SetParentId(FbxSceneNodeIndex, nodeNameToId[parent->GetName()]);
			}
			else
			{
				animation->SetParentId(FbxSceneNodeIndex, -1);
			}
		}
	}

	return animation;
}

void RFbxMeshLoader::LoadFbxMaterials(FbxNode* SceneNode, vector<RMaterial>& OutMaterials) const
{
	int NumFbxMaterials = SceneNode->GetSrcObjectCount<FbxSurfaceMaterial>();

	for (int IdxMaterial = 0; IdxMaterial < NumFbxMaterials; IdxMaterial++)
	{
		RMaterial meshMaterial = { 0 };
		FbxSurfaceMaterial* material = SceneNode->GetSrcObject<FbxSurfaceMaterial>(IdxMaterial);

		const char* texType[] =
		{
			FbxSurfaceMaterial::sDiffuse,
			FbxSurfaceMaterial::sNormalMap,
			FbxSurfaceMaterial::sSpecular,
		};

		for (int idxTexProp = 0; idxTexProp < 3; idxTexProp++)
		{
			FbxProperty prop = material->FindProperty(texType[idxTexProp]);
			int layeredTexCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
			string textureName;

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

				string ddsFilename = RFileUtil::ReplaceExtension(textureName, "dds");
				RTexture* texture = RResourceManager::Instance().FindTexture(ddsFilename.data());

				if (!texture)
				{
					texture = RResourceManager::Instance().LoadDDSTexture(RResourceManager::GetResourcePath(ddsFilename).data(), EResourceLoadMode::Immediate);
				}

				meshMaterial.Textures[meshMaterial.TextureNum] = texture;
				meshMaterial.TextureNum++;
			}
		}

		OutMaterials.push_back(meshMaterial);
	}

	if (NumFbxMaterials == 0)
	{
		OutMaterials.push_back(RMaterial{ 0 });
	}
}

void RFbxMeshLoader::OptimizeMesh(vector<UINT>& IndexData, vector<RVertexType::MeshLoader>& VertexData) const
{
	// Optimize mesh
	RLog("Optimizing mesh...\n");

	map<RVertexType::MeshLoader, int> meshVertIndexTable;
	vector<RVertexType::MeshLoader> optimizedVertData;
	vector<UINT> optimizedIndexData;
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

void RFbxMeshLoader::MatrixTransfer(RMatrix4* dest, const FbxAMatrix* src) const
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			dest->m[y][x] = (float)src->Get(y, x);
		}
	}
}
