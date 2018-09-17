#include "stdafx.h"
#include "ManagedSceneObject.h"

namespace ManagedEngineWrapper {

#pragma managed(push, off)

	FORCEINLINE void GetObjectPositionInFloat3(RSceneObject* SceneObject, float& x, float& y, float& z)
	{
		RVec3 Position = SceneObject->GetPosition();
		x = Position.X();
		y = Position.Y();
		z = Position.Z();
	}

	FORCEINLINE void SetObjectPositionInFloat3(RSceneObject* SceneObject, float x, float y, float z)
	{
		SceneObject->SetPosition(RVec3(x, y, z));
	}

	FORCEINLINE void GetObjectRotationInFloat3(RSceneObject* SceneObject, float& x, float& y, float& z)
	{
		RVec3 Euler = SceneObject->GetRotation().ToEuler();
		x = RMath::RadianToDegree(Euler.X());
		y = RMath::RadianToDegree(Euler.Y());
		z = RMath::RadianToDegree(Euler.Z());
	}

	FORCEINLINE void SetObjectRotationInFloat3(RSceneObject* SceneObject, float x, float y, float z)
	{
		SceneObject->SetRotation(RQuat::Euler(RMath::DegreeToRadian(x), RMath::DegreeToRadian(y), RMath::DegreeToRadian(z)));
	}

	FORCEINLINE void GetObjectScaleInFloat3(RSceneObject* SceneObject, float& x, float& y, float& z)
	{
		RVec3 Scale = SceneObject->GetScale();
		x = Scale.X();
		y = Scale.Y();
		z = Scale.Z();
	}

	FORCEINLINE void SetObjectScaleInFloat3(RSceneObject* SceneObject, float x, float y, float z)
	{
		SceneObject->SetScale(RVec3(x, y, z));
	}

#pragma managed(pop)

	ManagedMaterial::ManagedMaterial(RMaterial* mat, const char* elemName)
		: material(mat), meshElementName(gcnew String(elemName))
	{
		
	}

	ManagedMaterialCollection::ManagedMaterialCollection(RSMeshObject* obj)
	{
		for (int i = 0; i < obj->GetMeshElementCount(); i++)
		{
			materials.Add(gcnew ManagedMaterial(obj->GetMaterial(i), obj->GetMesh()->GetMeshElements()[i].GetName().c_str()));
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

	String^ ManagedSceneObject::Float3ToString(float x, float y, float z)
	{
		return String::Format(L"{0}, {1}, {2}", x, y, z);
	}

	void ManagedSceneObject::StringToFloat3(String^ str, float& x, float &y, float &z)
	{
		String^ delimStr = ",";
		cli::array<Char>^ delimiter = delimStr->ToCharArray();
		cli::array<String^>^ words = str->Split(delimiter);

		x = (float)Convert::ToDouble(words[0]);
		y = (float)Convert::ToDouble(words[1]);
		z = (float)Convert::ToDouble(words[2]);
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
			matList->Add(gcnew ManagedMaterial(GetMeshObject()->GetMaterial(i), GetMeshObject()->GetMesh()->GetMeshElements()[i].GetName().c_str()));
		}
		return matList;
	}

	String^ ManagedMeshObject::VertexComponents::get()
	{
		RSMeshObject* MeshObject = GetMeshObject();

		if (MeshObject)
		{
			RMesh* mesh = MeshObject->GetMesh();

			if (mesh && mesh->GetMeshElementCount())
			{
				return gcnew String(RVertexDeclaration::GetVertexComponentsString(mesh->GetMeshElements()[0].GetVertexComponentMask()).c_str());
			}
		}

		return gcnew String("");
	}

	System::String^ ManagedSceneObject::Position::get()
	{
		float x, y, z;
		GetObjectPositionInFloat3(m_SceneObject, x, y, z);
		return Float3ToString(x, y, z);
	}

	void ManagedSceneObject::Position::set(String^ value)
	{
		float x, y, z;
		StringToFloat3(value, x, y, z);
		SetObjectPositionInFloat3(m_SceneObject, x, y, z);
	}

	System::String^ ManagedSceneObject::Rotation::get()
	{
		float x, y, z;
		GetObjectRotationInFloat3(m_SceneObject, x, y, z);
		return Float3ToString(x, y, z);
	}

	void ManagedSceneObject::Rotation::set(String^ value)
	{
		float x, y, z;
		StringToFloat3(value, x, y, z);
		SetObjectRotationInFloat3(m_SceneObject, x, y, z);
	}

	System::String^ ManagedSceneObject::Scale::get()
	{
		float x, y, z;
		GetObjectScaleInFloat3(m_SceneObject, x, y, z);
		return Float3ToString(x, y, z);
	}

	void ManagedSceneObject::Scale::set(String^ value)
	{
		float x, y, z;
		StringToFloat3(value, x, y, z);
		SetObjectScaleInFloat3(m_SceneObject, x, y, z);
	}

}