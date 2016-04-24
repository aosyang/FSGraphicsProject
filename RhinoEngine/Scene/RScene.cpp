//=============================================================================
// RScene.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RScene.h"

#include "../tinyxml2/tinyxml2.h"

void RScene::Initialize()
{
	cbPerObject.Initialize();
	cbScene.Initialize();
	cbBoneMatrices.Initialize();
	cbLight.Initialize();
	cbMaterial.Initialize();
	cbScreen.Initialize();
}

void RScene::Release()
{
	RemoveAllObjects();

	cbPerObject.Release();
	cbScene.Release();
	cbBoneMatrices.Release();
	cbLight.Release();
	cbMaterial.Release();
	cbScreen.Release();
}

RSMeshObject* RScene::CreateMeshObject(const char* meshName)
{
	RMesh* mesh = RResourceManager::Instance().FindMesh(meshName);
	assert(mesh);

	return CreateMeshObject(mesh);
}

RSMeshObject* RScene::CreateMeshObject(RMesh* mesh)
{
	RSMeshObject* meshObject = new RSMeshObject();
	meshObject->SetScene(this);
	meshObject->SetMesh(mesh);

	m_SceneObjects.push_back(meshObject);

	return meshObject;
}

void RScene::RemoveAllObjects()
{
	for (UINT i = 0; i < m_SceneObjects.size(); i++)
	{
		delete m_SceneObjects[i];
	}

	m_SceneObjects.clear();
}

void RScene::LoadFromFile(const char* filename)
{
	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
	if (doc->LoadFile(filename) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement* root = doc->RootElement();
		tinyxml2::XMLElement* elem_obj = root->FirstChildElement("SceneObject");
		while (elem_obj)
		{
			tinyxml2::XMLElement* elem_transform = elem_obj->FirstChildElement("Transform");
			RMatrix4 transform;
			int n = 0;
			stringstream ss(elem_transform->GetText());

			while (ss >> ((float*)&transform)[n++] && n < 16)
			{
				if (ss.peek() == ',')
					ss.ignore();
			}

			string obj_type = elem_obj->Attribute("Type");
			if (obj_type == "MeshObject")
			{
				const char* resPath = elem_obj->Attribute("Mesh");
				RMesh* mesh = RResourceManager::Instance().FindMesh(resPath);

				if (!mesh)
				{
					mesh = RResourceManager::Instance().LoadFbxMesh(resPath, RLM_Immediate);
				}

				RSMeshObject* meshObj = CreateMeshObject(resPath);
				meshObj->SetTransform(transform);
			}

			elem_obj = elem_obj->NextSiblingElement();
		}
	}
}

void RScene::SaveToFile(const char* filename)
{
	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
	tinyxml2::XMLElement* elem_scene = doc->NewElement("Scene");

	for (vector<RSceneObject*>::iterator iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); iter++)
	{
		tinyxml2::XMLElement* elem_obj = doc->NewElement("SceneObject");

		if ((*iter)->GetType() == SO_MeshObject)
		{
			elem_obj->SetAttribute("Type", "MeshObject");

			RSMeshObject* meshObj = (RSMeshObject*)(*iter);
			elem_obj->SetAttribute("Mesh", meshObj->GetMesh()->GetPath().c_str());
		}

		const RMatrix4& t = (*iter)->GetNodeTransform();

		tinyxml2::XMLElement* elem_trans = doc->NewElement("Transform");

		char msg_buf[512];
		sprintf_s(msg_buf, 512,
					"%f, %f, %f, %f, "
					"%f, %f, %f, %f, "
					"%f, %f, %f, %f, "
					"%f, %f, %f, %f",
					t._m11, t._m12, t._m13, t._m14,
					t._m21, t._m22, t._m23, t._m24, 
					t._m31, t._m32, t._m33, t._m34, 
					t._m41, t._m42, t._m43, t._m44 );

		elem_trans->SetText(msg_buf);
		elem_obj->InsertEndChild(elem_trans);
		elem_scene->InsertEndChild(elem_obj);
	}

	doc->InsertEndChild(elem_scene);
	doc->SaveFile(filename);
}

void RScene::Render(const RFrustum* pFrustum)
{
	for (vector<RSceneObject*>::iterator iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); iter++)
	{
		if (pFrustum && !RCollision::TestAabbInsideFrustum(*pFrustum, (*iter)->GetAabb()))
			continue;

		SHADER_OBJECT_BUFFER cbObject;
		cbObject.worldMatrix = (*iter)->GetNodeTransform();
		cbPerObject.UpdateContent(&cbObject);
		cbPerObject.ApplyToShaders();
		(*iter)->Draw();
	}
}

vector<RSceneObject*>& RScene::GetSceneObjects()
{
	return m_SceneObjects;
}
