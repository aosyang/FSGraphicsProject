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
	cbGlobal.Initialize();
}

void RScene::Release()
{
	DestroyAllObjects();

	cbPerObject.Release();
	cbScene.Release();
	cbBoneMatrices.Release();
	cbLight.Release();
	cbMaterial.Release();
	cbGlobal.Release();
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

RSceneObject* RScene::CloneObject(RSceneObject* obj)
{
	return obj->Clone();
}

RSceneObject* RScene::FindObject(const char* name) const
{
	for (UINT i = 0; i < m_SceneObjects.size(); i++)
	{
		if (m_SceneObjects[i]->GetName() == name)
			return m_SceneObjects[i];
	}

	return nullptr;
}

bool RScene::AddObjectToScene(RSceneObject* obj)
{
	vector<RSceneObject*>::iterator iter = find(m_SceneObjects.begin(), m_SceneObjects.end(), obj);
	if (iter == m_SceneObjects.end())
	{
		m_SceneObjects.push_back(obj);
		return true;
	}

	return false;
}

void RScene::RemoveObjectFromScene(RSceneObject* obj)
{
	vector<RSceneObject*>::iterator iter = find(m_SceneObjects.begin(), m_SceneObjects.end(), obj);
	if (iter != m_SceneObjects.end())
	{
		(*iter)->SetScene(nullptr);
		m_SceneObjects.erase(iter);
	}
}

void RScene::DestroyObject(RSceneObject* obj)
{
	vector<RSceneObject*>::iterator iter = find(m_SceneObjects.begin(), m_SceneObjects.end(), obj);
	if (iter != m_SceneObjects.end())
	{
		m_SceneObjects.erase(iter);
		delete obj;
	}
}

void RScene::DestroyAllObjects()
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
			const char* name = elem_obj->Attribute("Name");
			const char* script = elem_obj->Attribute("Script");

			tinyxml2::XMLElement* elem_transform = elem_obj->FirstChildElement("Transform");
			RMatrix4 transform;
			int n = 0;
			stringstream ss(elem_transform->GetText());

			while (ss >> ((float*)&transform)[n++] && n < 16)
			{
				if (ss.peek() == ',')
					ss.ignore();
			}

			RSceneObject* sceneObj = nullptr;

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
				sceneObj = meshObj;

				tinyxml2::XMLElement* elem_mat = elem_obj->FirstChildElement("Material");
				vector<RMaterial> xmlMaterials;

				if (elem_mat)
				{
					for (int i = 0; i < meshObj->GetMeshElementCount(); i++)
					{
						xmlMaterials.push_back(*meshObj->GetMaterial(i));
					}
				}

				while (elem_mat)
				{
					int index = elem_mat->IntAttribute("Index");

					tinyxml2::XMLElement* elem = elem_mat->FirstChildElement("MeshElement");
					while (elem)
					{
						const char* shaderName = elem->Attribute("Shader");
						RMaterial material = { nullptr, 0 };
						material.Shader = RShaderManager::Instance().GetShaderResource(shaderName);

						tinyxml2::XMLElement* elem_tex = elem->FirstChildElement();
						while (elem_tex)
						{
							const char* textureName = elem_tex->GetText();

							RTexture* texture = RResourceManager::Instance().FindTexture(textureName);

							if (!texture)
							{
								texture = RResourceManager::Instance().LoadDDSTexture(RResourceManager::GetResourcePath(textureName).data(), RLM_Immediate);
							}

							material.Textures[material.TextureNum++] = texture;
							elem_tex = elem_tex->NextSiblingElement();
						}

						xmlMaterials[index] = material;
						elem = elem->NextSiblingElement();
					}

					elem_mat = elem_mat->NextSiblingElement("Material");
				}

				if (xmlMaterials.size())
				{
					meshObj->SetMaterial(xmlMaterials.data(), (int)xmlMaterials.size());
				}
			}

			if (name)
				sceneObj->SetName(name);

			if (script)
				sceneObj->SetScript(script);

			elem_obj = elem_obj->NextSiblingElement();
		}
	}

	delete doc;
}

void RScene::SaveToFile(const char* filename)
{
	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
	tinyxml2::XMLElement* elem_scene = doc->NewElement("Scene");

	for (vector<RSceneObject*>::iterator iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); iter++)
	{
		tinyxml2::XMLElement* elem_obj = doc->NewElement("SceneObject");

		if ((*iter)->GetName() != "")
		{
			elem_obj->SetAttribute("Name", (*iter)->GetName().c_str());
		}

		if ((*iter)->GetScript() != "")
		{
			elem_obj->SetAttribute("Script", (*iter)->GetScript().c_str());
		}

		if ((*iter)->GetType() == SO_MeshObject)
		{
			elem_obj->SetAttribute("Type", "MeshObject");

			RSMeshObject* meshObj = (RSMeshObject*)(*iter);
			RMesh* mesh = meshObj->GetMesh();
			elem_obj->SetAttribute("Mesh", mesh->GetPath().c_str());

			// Save materials
			for (int i = 0; i < meshObj->GetMeshElementCount(); i++)
			{
				RMaterial meshMaterial = mesh->GetMaterial(i);
				RMaterial* objMaterial = meshObj->GetMaterial(i);
				bool exportMaterial = false;

				if (meshMaterial.Shader != objMaterial->Shader ||
					meshMaterial.TextureNum != objMaterial->TextureNum)
				{
					exportMaterial = true;
				}

				if (!exportMaterial)
				{
					for (int j = 0; j < meshMaterial.TextureNum; j++)
					{
						if (meshMaterial.Textures[j] != objMaterial->Textures[j])
						{
							exportMaterial = true;
							break;
						}
					}
				}

				if (exportMaterial)
				{
					tinyxml2::XMLElement* elem_mat = doc->NewElement("Material");
					elem_mat->SetAttribute("Index", i);
					meshObj->SerializeMaterialsToXML(doc, elem_mat);
					elem_obj->InsertEndChild(elem_mat);
				}
			}
		}

		// Save transformation
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

RVec3 RScene::TestMovingAabbWithScene(const RAabb& aabb, const RVec3& moveVec)
{
	RVec3 v = moveVec;

	for (vector<RSceneObject*>::iterator iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); iter++)
	{
		if ((*iter)->GetType() == SO_MeshObject)
		{
			RSMeshObject* meshObj = (RSMeshObject*)(*iter);

			if (aabb.GetSweptAabb(v).TestIntersectionWithAabb(meshObj->GetAabb()))
			{
				for (int i = 0; i < meshObj->GetMeshElementCount(); i++)
				{
					RAabb elemAabb = meshObj->GetMeshElementAabb(i).GetTransformedAabb(meshObj->GetNodeTransform());
					v = aabb.TestDynamicCollisionWithAabb(v, elemAabb);
				}
			}
		}
	}

	return v;
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

void RScene::RenderDepthPass(const RFrustum* pFrustum)
{
	for (vector<RSceneObject*>::iterator iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); iter++)
	{
		if (pFrustum && !RCollision::TestAabbInsideFrustum(*pFrustum, (*iter)->GetAabb()))
			continue;

		SHADER_OBJECT_BUFFER cbObject;
		cbObject.worldMatrix = (*iter)->GetNodeTransform();
		cbPerObject.UpdateContent(&cbObject);
		cbPerObject.ApplyToShaders();
		(*iter)->DrawDepthPass();
	}
}

vector<RSceneObject*>& RScene::GetSceneObjects()
{
	return m_SceneObjects;
}
