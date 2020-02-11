//=============================================================================
// RScene.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RScene.h"

#include "../tinyxml2/tinyxml2.h"

// If set to 1, rotations saved in local files are in degrees instead of radians
#define SAVE_ROTATION_IN_DEGREES 0

// XML helper functions
namespace
{
	bool XmlReadObjectTransform(tinyxml2::XMLElement* ObjectElement, RVec3& OutPosition, RQuat& OutRotation, RVec3& OutScale)
	{
		if (ObjectElement)
		{
			tinyxml2::XMLElement* XmlElementTransform = ObjectElement->FirstChildElement("Transform");
			if (XmlElementTransform)
			{
				int ElementNums = 0;

				RVec3 Position(0.0f, 0.0f, 0.0f);
				RQuat Rotation = RQuat::IDENTITY;
				RVec3 Scale = RVec3(1.0f, 1.0f, 1.0f);

				tinyxml2::XMLElement* XmlElementPosition = XmlElementTransform->FirstChildElement("Position");
				if (XmlElementPosition)
				{
					float PosData[3];
					std::stringstream StringStream(XmlElementPosition->GetText());

					for (int n = 0; n < 3; n++)
					{
						StringStream >> PosData[n];

						// Ignore remaining commas in the input string
						if (StringStream.peek() == ',')
						{
							StringStream.ignore();
						}
					}

					Position = RVec3(PosData);
					ElementNums++;
				}

				tinyxml2::XMLElement* XmlElementRotation = XmlElementTransform->FirstChildElement("Rotation");
				if (XmlElementRotation)
				{
					float RotData[3];
					std::stringstream StringStream(XmlElementRotation->GetText());

					for (int n = 0; n < 3; n++)
					{
						StringStream >> RotData[n];

						// Ignore remaining commas in the input string
						if (StringStream.peek() == ',')
						{
							StringStream.ignore();
						}

#if (SAVE_ROTATION_IN_DEGREES == 1)
						RotData[n] = RMath::DegreeToRadian(RotData[n]);
#endif	// (SAVE_ROTATION_IN_DEGREES == 1)
					}

					Rotation = RQuat::Euler(RVec3(RotData));
					ElementNums++;
				}

				tinyxml2::XMLElement* XmlElementScale = XmlElementTransform->FirstChildElement("Scale");
				if (XmlElementScale)
				{
					float ScaleData[3];
					std::stringstream StringStream(XmlElementScale->GetText());

					for (int n = 0; n < 3; n++)
					{
						StringStream >> ScaleData[n];

						// Ignore remaining commas in the input string
						if (StringStream.peek() == ',')
						{
							StringStream.ignore();
						}
					}

					Scale = RVec3(ScaleData);
					ElementNums++;
				}

				if (ElementNums > 0)
				{
					OutPosition = Position;
					OutRotation = Rotation;
					OutScale = Scale;

					return true;
				}
			}
		}

		return false;
	}

	void XmlWriteObjectTransform(tinyxml2::XMLElement* ObjectElement, RSceneObject* SceneObject)
	{
		if (SceneObject && ObjectElement)
		{
			tinyxml2::XMLDocument* XmlDocument = ObjectElement->GetDocument();
			if (XmlDocument)
			{
				tinyxml2::XMLElement* XmlElementTransform = XmlDocument->NewElement("Transform");
				if (XmlElementTransform)
				{
					tinyxml2::XMLElement* XmlElementPosition = XmlDocument->NewElement("Position");
					if (XmlElementPosition)
					{
						RVec3 Position = SceneObject->GetPosition();

						char msg_buf[512];
						sprintf_s(msg_buf, sizeof(msg_buf), "%f, %f, %f", Position.X(), Position.Y(), Position.Z());
						XmlElementPosition->SetText(msg_buf);
						XmlElementTransform->InsertEndChild(XmlElementPosition);
					}

					tinyxml2::XMLElement* XmlElementRotation = XmlDocument->NewElement("Rotation");
					if (XmlElementRotation)
					{
						RVec3 EulerAngles = SceneObject->GetRotation().ToEuler();

						char msg_buf[512];
						sprintf_s(msg_buf, sizeof(msg_buf), "%f, %f, %f",
#if (SAVE_ROTATION_IN_DEGREES == 1)
							RMath::RadianToDegree(EulerAngles.X()),
							RMath::RadianToDegree(EulerAngles.Y()),
							RMath::RadianToDegree(EulerAngles.Z())
#else
							EulerAngles.X(), EulerAngles.Y(), EulerAngles.Z()
#endif	// (SAVE_ROTATION_IN_DEGREES == 1)
						);
						XmlElementRotation->SetText(msg_buf);
						XmlElementTransform->InsertEndChild(XmlElementRotation);
					}

					tinyxml2::XMLElement* XmlElementScale = XmlDocument->NewElement("Scale");
					if (XmlElementScale)
					{
						RVec3 Scale = SceneObject->GetScale();

						char msg_buf[512];
						sprintf_s(msg_buf, sizeof(msg_buf), "%f, %f, %f", Scale.X(), Scale.Y(), Scale.Z());
						XmlElementScale->SetText(msg_buf);
						XmlElementTransform->InsertEndChild(XmlElementScale);
					}

					ObjectElement->InsertEndChild(XmlElementTransform);
				}
			}
		}
	}

	bool XmlReadObjectTransformAsMatrix(tinyxml2::XMLElement* ObjectElement, RMatrix4& OutMatrix)
	{
		if (ObjectElement)
		{
			tinyxml2::XMLElement* XmlElementTransform = ObjectElement->FirstChildElement("Transform");
			if (XmlElementTransform)
			{
				RMatrix4 Matrix;
				std::stringstream StringStream(XmlElementTransform->GetText());

				for (int n = 0; n < 16; n++)
				{
					StringStream >> ((float*)&Matrix)[n];

					// Ignore remaining commas in the input string
					if (StringStream.peek() == ',')
					{
						StringStream.ignore();
					}
				}

				OutMatrix = Matrix;
				return true;
			}
		}

		return false;
	}

	void XmlWriteObjectTransformAsMatrix(tinyxml2::XMLElement* ObjectElement, RSceneObject* SceneObject)
	{
		if (SceneObject && ObjectElement)
		{
			tinyxml2::XMLDocument* XmlDocument = ObjectElement->GetDocument();
			if (XmlDocument)
			{
				tinyxml2::XMLElement* XmlElementTransform = XmlDocument->NewElement("Transform");
				if (XmlElementTransform)
				{
					// Save transformation
					const RMatrix4& t = SceneObject->GetTransformMatrix();

					char msg_buf[512];
					sprintf_s(msg_buf, sizeof(msg_buf),
						"%f, %f, %f, %f, "
						"%f, %f, %f, %f, "
						"%f, %f, %f, %f, "
						"%f, %f, %f, %f",
						t._m11, t._m12, t._m13, t._m14,
						t._m21, t._m22, t._m23, t._m24,
						t._m31, t._m32, t._m33, t._m34,
						t._m41, t._m42, t._m43, t._m44);

					XmlElementTransform->SetText(msg_buf);
					ObjectElement->InsertEndChild(XmlElementTransform);
				}
			}
		}
	}
}

RScene::RScene()
	: m_RenderCamera(nullptr)
{

}

void RScene::Initialize()
{
	if (GRenderer.GetActiveScene() == nullptr)
	{
		GRenderer.SetActiveScene(this);
	}
}

void RScene::Release()
{
	DestroyAllObjects();
}

RSMeshObject* RScene::CreateMeshObject(const char* meshPath)
{
	RMesh* mesh = RResourceManager::Instance().FindResource<RMesh>(meshPath);
	if (!mesh)
	{
		mesh = RResourceManager::Instance().LoadResource<RMesh>(meshPath, EResourceLoadMode::Immediate);
	}
	assert(mesh);

	return CreateMeshObject(mesh);
}

RSMeshObject* RScene::CreateMeshObject(RMesh* mesh)
{
	RSMeshObject* meshObject = new RSMeshObject(RConstructingParams(this));
	meshObject->SetMesh(mesh);

	AddSceneObjectInternal(meshObject);

	return meshObject;
}

RSceneObject* RScene::CreateSceneObject(const char* name)
{
	return CreateSceneObjectOfType<RSceneObject>(name);
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

std::string RScene::GenerateUniqueObjectName(const std::string& ObjectName)
{
	std::string UniqueName;
	int NameIndex = 0;
	while (true)
	{
		UniqueName = ObjectName + "_" + std::to_string(NameIndex);
		NameIndex++;

		if (!DoesObjectNameExist(UniqueName))
		{
			break;
		}
	}

	return UniqueName;
}

std::string RScene::GenerateUniqueObjectNameForClone(const std::string& ObjectName)
{
	size_t UnderScorePos = ObjectName.find_last_of('_');

	// Object name doesn't contain under score, simply generate a unique name.
	if (UnderScorePos != std::string::npos)
	{
		bool bHasDigitSuffix = true;

		std::string Suffix = ObjectName.substr(UnderScorePos + 1);
		for (size_t i = 0; i < Suffix.length(); i++)
		{
			if (!isdigit(Suffix[i]))
			{
				bHasDigitSuffix = false;
				break;
			}
		}

		if (bHasDigitSuffix)
		{
			std::string BaseName = ObjectName.substr(0, UnderScorePos);
			return GenerateUniqueObjectName(BaseName);
		}
	}

	return GenerateUniqueObjectName(ObjectName);
}

bool RScene::DoesObjectNameExist(const std::string& Name) const
{
	for (auto SceneObject : m_SceneObjects)
	{
		if (SceneObject->GetName() == Name)
		{
			return true;
		}
	}

	return false;
}

void RScene::DestroyObject(RSceneObject* obj)
{
	auto iter = find(m_SceneObjects.begin(), m_SceneObjects.end(), obj);
	if (iter != m_SceneObjects.end())
	{
		(*iter)->Release();
		m_SceneObjects.erase(iter);
		delete obj;
	}
}

void RScene::DestroyAllObjects()
{
	for (UINT i = 0; i < m_SceneObjects.size(); i++)
	{
		m_SceneObjects[i]->Release();
		delete m_SceneObjects[i];
	}

	m_SceneObjects.clear();
}

void RScene::LoadFromFile(const std::string& MapAssetPath)
{
	const std::string MapFilePath = RFileUtil::CombinePath(RResourceManager::GetAssetsBasePath(), MapAssetPath);

	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
	if (doc->LoadFile(MapFilePath.c_str()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement* root = doc->RootElement();
		tinyxml2::XMLElement* elem_obj = root->FirstChildElement("SceneObject");
		while (elem_obj)
		{
			RSceneObject* NewSceneObject = nullptr;

			std::string obj_type = elem_obj->Attribute("Type");
			if (obj_type == "MeshObject")
			{
				const char* ResourceFilePath = elem_obj->Attribute("Mesh");
				RMesh* mesh = RResourceManager::Instance().FindResource<RMesh>(ResourceFilePath);

				if (!mesh)
				{
					mesh = RResourceManager::Instance().LoadResource<RMesh>(ResourceFilePath, EResourceLoadMode::Immediate);
				}

				RSMeshObject* MeshObject = CreateMeshObject(ResourceFilePath);
				NewSceneObject = MeshObject;

				tinyxml2::XMLElement* elem_mat = elem_obj->FirstChildElement("Material");
				MeshObject->SerializeXmlMaterials_Load(elem_mat);
			}

			RVec3 ObjectPosition;
			RQuat ObjectRotation;
			RVec3 ObjectScale;

			if (!XmlReadObjectTransform(elem_obj, ObjectPosition, ObjectRotation, ObjectScale))
			{
				RMatrix4 Matrix;
				
				// Failed to read transform as separated components, fallback to reading transform as matrix
				if (!XmlReadObjectTransformAsMatrix(elem_obj, Matrix))
				{
					// If transform element is not found, use identity matrix for the object's transform
					Matrix = RMatrix4::IDENTITY;
				}

				// Decompose transform from matrix
				Matrix.Decompose(ObjectPosition, ObjectRotation, ObjectScale);
			}

			NewSceneObject->SetTransform(ObjectPosition, ObjectRotation, ObjectScale);

			const char* ObjectName = elem_obj->Attribute("Name");
			if (ObjectName)
			{
				NewSceneObject->SetName(ObjectName);
			}

			const char* ObjectScript = elem_obj->Attribute("Script");
			if (ObjectScript)
			{
				NewSceneObject->SetScript(ObjectScript);
			}

			elem_obj = elem_obj->NextSiblingElement();
		}
	}

	delete doc;
}

void RScene::SaveToFile(const char* filename)
{
	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument();
	doc->InsertEndChild(doc->NewDeclaration());
	tinyxml2::XMLElement* elem_scene = doc->NewElement("Scene");

	for (auto* SceneObject : m_SceneObjects)
	{
		if (SceneObject->HasFlags(CF_NoSerialization))
		{
			continue;
		}

		tinyxml2::XMLElement* elem_obj = doc->NewElement("SceneObject");

		if (SceneObject->GetName() != "")
		{
			elem_obj->SetAttribute("Name", SceneObject->GetName().c_str());
		}

		if (SceneObject->GetScript() != "")
		{
			elem_obj->SetAttribute("Script", SceneObject->GetScript().c_str());
		}

		if (SceneObject->CanCastTo<RSMeshObject>())
		{
			elem_obj->SetAttribute("Type", "MeshObject");

			RSMeshObject* meshObj = (RSMeshObject*)SceneObject;
			RMesh* mesh = meshObj->GetMesh();
			elem_obj->SetAttribute("Mesh", mesh->GetAssetPath().c_str());

			// Save materials
			for (int i = 0; i < meshObj->GetMeshElementCount(); i++)
			{
				const RMaterial* meshMaterial = mesh->GetMaterial(i);
				RMaterial* objMaterial = meshObj->GetMaterial(i);
				bool exportMaterial = false;

				if (meshMaterial != objMaterial)
				{
					exportMaterial = true;
				}

				if (exportMaterial)
				{
					tinyxml2::XMLElement* elem_mat = doc->NewElement("Material");
					elem_mat->SetAttribute("Index", i);
					meshObj->SerializeXmlMaterials_Save(doc, elem_mat);
					elem_obj->InsertEndChild(elem_mat);
				}
			}
		}

		XmlWriteObjectTransform(elem_obj, SceneObject);
		elem_scene->InsertEndChild(elem_obj);
	}

	doc->InsertEndChild(elem_scene);
	doc->SaveFile(filename);
}

void RScene::SetRenderCamera(RCamera* Camera)
{
	m_RenderCamera = Camera;
}

RCamera* RScene::GetRenderCamera() const
{
	return m_RenderCamera;
}

void RScene::NotifyCameraCreated(RCamera* Camera)
{
	// If new camera is the first one in the scene, use it as render camera
	if (m_RenderCamera == nullptr)
	{
		m_RenderCamera = Camera;
	}
}

void RScene::NotifyCameraDestroying(RCamera* Camera)
{
	if (m_RenderCamera == Camera)
	{
		m_RenderCamera = nullptr;
	}
}

RVec3 RScene::TestMovingAabbWithScene(const RAabb& aabb, const RVec3& moveVec, std::list<RSceneObject*> IgnoredObjects /*= std::list<RSceneObject*>()*/)
{
	RVec3 v = moveVec;

	for (auto SceneObject : m_SceneObjects)
	{
		if (SceneObject->CanCastTo<RSMeshObject>())
		{
			if (!StdContains(IgnoredObjects, SceneObject))
			{
				RSMeshObject* meshObj = (RSMeshObject*)(SceneObject);

				if (aabb.GetSweptAabb(v).TestIntersectionWithAabb(meshObj->GetAabb()))
				{
					for (int i = 0; i < meshObj->GetMeshElementCount(); i++)
					{
						RAabb elemAabb = meshObj->GetMeshElementAabb(i).GetTransformedAabb(meshObj->GetTransformMatrix());
						v = aabb.TestDynamicCollisionWithAabb(v, elemAabb);
					}
				}
			}
		}
	}

	return v;
}

void RScene::Render(const RFrustum* pFrustum)
{
	for (auto SceneObject : m_SceneObjects)
	{
		if (pFrustum && !RCollision::TestAabbInsideFrustum(*pFrustum, SceneObject->GetAabb()))
		{
			continue;
		}

		if (!SceneObject->IsVisible())
		{
			continue;
		}

		RConstantBuffers::cbPerObject.Data.worldMatrix = SceneObject->GetTransformMatrix();
		RConstantBuffers::cbPerObject.UpdateBufferData();
		RConstantBuffers::cbPerObject.BindBuffer();
		SceneObject->Draw();
	}
}

void RScene::RenderDepthPass(const RFrustum* pFrustum)
{
	for (auto SceneObject : m_SceneObjects)
	{
		if (pFrustum && !RCollision::TestAabbInsideFrustum(*pFrustum, SceneObject->GetAabb()))
		{
			continue;
		}

		if (!SceneObject->IsVisible())
		{
			continue;
		}

		RConstantBuffers::cbPerObject.Data.worldMatrix = SceneObject->GetTransformMatrix();
		RConstantBuffers::cbPerObject.UpdateBufferData();
		RConstantBuffers::cbPerObject.BindBuffer();
		SceneObject->DrawDepthPass();
	}
}

void RScene::UpdateScene(float DeltaTime)
{
	for (RSceneObject* SceneObject : m_SceneObjects)
	{
		SceneObject->Update(DeltaTime);
	}
}

std::vector<RSceneObject*> RScene::EnumerateSceneObjects() const
{
	std::vector<RSceneObject*> OutputObjects;

	for (auto& SceneObject : m_SceneObjects)
	{
		if (SceneObject->HasFlags(CF_InternalObject))
		{
			continue;
		}

		OutputObjects.push_back(SceneObject);
	}

	return OutputObjects;
}

void RScene::AddSceneObjectInternal(RSceneObject* SceneObject)
{
	assert(!StdContains(m_SceneObjects, SceneObject));
	m_SceneObjects.push_back(SceneObject);
}
