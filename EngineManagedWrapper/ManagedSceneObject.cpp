#include "stdafx.h"
#include "ManagedSceneObject.h"

namespace EngineManagedWrapper {

	ManagedMaterial::ManagedMaterial(RMaterial* mat)
		: material(mat)
	{
		
	}

	ManagedMaterialCollection::ManagedMaterialCollection(RSMeshObject* obj)
	{
		for (int i = 0; i < obj->GetMeshElementCount(); i++)
		{
			materials.Add(gcnew ManagedMaterial(obj->GetMaterial(i)));
		}
	}

	ManagedSceneObject::ManagedSceneObject(RSceneObject* obj)
		: m_SceneObject(obj)
	{
	}

	bool ManagedSceneObject::IsValid()
	{
		return m_SceneObject != NULL;
	}

	String^ ManagedSceneObject::Vec3ToString(const RVec3& vec)
	{
		return String::Format(L"{0}, {1}, {2}", vec.x, vec.y, vec.z);
	}

	RVec3 ManagedSceneObject::StringToVec3(String^ str)
	{
		String^ delimStr = ",";
		array<Char>^ delimiter = delimStr->ToCharArray();
		array<String^>^ words = str->Split(delimiter);

		RVec3 vec;
		vec.x = (float)Convert::ToDouble(words[0]);
		vec.y = (float)Convert::ToDouble(words[1]);
		vec.z = (float)Convert::ToDouble(words[2]);

		return vec;
	}

	ManagedMeshObject::ManagedMeshObject(RSceneObject* obj)
		: ManagedSceneObject(obj)
	{
	}

	String^ ManagedMeshObject::Asset::get()
	{
		RMesh* mesh = GetMeshObject()->GetMesh();
		if (mesh)
			return gcnew String(mesh->GetPath().c_str());
		else
			return gcnew String("");
	}

	void ManagedMeshObject::Asset::set(String^ value)
	{
		RMesh* mesh = RResourceManager::Instance().FindMesh(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer()));
		if (mesh)
			GetMeshObject()->SetMesh(mesh);
	}

	List<ManagedMaterial^>^ ManagedMeshObject::Materials::get()
	{
		List<ManagedMaterial^>^ matList = gcnew List<ManagedMaterial^>();
		for (int i = 0; i < GetMeshObject()->GetMeshElementCount(); i++)
		{
			matList->Add(gcnew ManagedMaterial(GetMeshObject()->GetMaterial(i)));
		}
		return matList;
	}

	String^ ManagedMeshObject::VertexComponent::get()
	{
		RMesh* mesh = GetMeshObject()->GetMesh();
		if (!mesh || !mesh->GetMeshElementCount())
			return gcnew String("");

		return gcnew String(RVertexDeclaration::GetVertexComponentsString(mesh->GetMeshElements()[0].GetVertexComponentMask()).c_str());
	}

}