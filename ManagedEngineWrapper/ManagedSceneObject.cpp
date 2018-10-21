//=============================================================================
// ManagedSceneObject.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
//
//=============================================================================
#include "stdafx.h"
#include "ManagedSceneObject.h"
#include "TypeUtils.h"

namespace ManagedEngineWrapper
{

#pragma managed(push, off)

	FORCEINLINE void GetObjectPositionInFloat3(RSceneObject* SceneObject, float& x, float& y, float& z)
	{
		if (!SceneObject)
		{
			x = y = z = 0.0f;
			return;
		}

		RVec3 Position = SceneObject->GetPosition();
		x = Position.X();
		y = Position.Y();
		z = Position.Z();
	}

	FORCEINLINE void SetObjectPositionInFloat3(RSceneObject* SceneObject, float x, float y, float z)
	{
		if (SceneObject)
		{
			SceneObject->SetPosition(RVec3(x, y, z));
		}
	}

	FORCEINLINE void GetObjectRotationInFloat3(RSceneObject* SceneObject, float& x, float& y, float& z)
	{
		if (!SceneObject)
		{
			x = y = z = 0.0f;
			return;
		}

		RVec3 Euler = SceneObject->GetRotation().ToEuler();
		x = RMath::RadianToDegree(Euler.X());
		y = RMath::RadianToDegree(Euler.Y());
		z = RMath::RadianToDegree(Euler.Z());
	}

	FORCEINLINE void SetObjectRotationInFloat3(RSceneObject* SceneObject, float x, float y, float z)
	{
		if (SceneObject)
		{
			SceneObject->SetRotation(RQuat::Euler(RMath::DegreeToRadian(x), RMath::DegreeToRadian(y), RMath::DegreeToRadian(z)));
		}
	}

	FORCEINLINE void GetObjectScaleInFloat3(RSceneObject* SceneObject, float& x, float& y, float& z)
	{
		if (!SceneObject)
		{
			x = y = z = 0.0f;
			return;
		}

		RVec3 Scale = SceneObject->GetScale();
		x = Scale.X();
		y = Scale.Y();
		z = Scale.Z();
	}

	FORCEINLINE void SetObjectScaleInFloat3(RSceneObject* SceneObject, float x, float y, float z)
	{
		if (SceneObject)
		{
			SceneObject->SetScale(RVec3(x, y, z));
		}
	}

#pragma managed(pop)


	ManagedSceneObject::ManagedSceneObject(RSceneObject* obj)
		: m_SceneObject(obj)
	{
		// Bind position children value changed event
		CachedPosition.PropertyChanged += gcnew PropertyChangedEventHandler(this, &ManagedSceneObject::OnPositionChanged);
	}

	ManagedSceneObject::~ManagedSceneObject()
	{
		// Unbind position children value changed event
		CachedPosition.PropertyChanged -= gcnew PropertyChangedEventHandler(this, &ManagedSceneObject::OnPositionChanged);
	}

	bool ManagedSceneObject::IsValid()
	{
		return m_SceneObject != NULL;
	}

	RSceneObject* ManagedSceneObject::GetRawSceneObjectPtr()
	{
		return m_SceneObject;
	}

	String^ ManagedSceneObject::DisplayName::get()
	{
		if (Name != "")
		{
			return Name;
		}

		return "[Unnamed]";
	}

	void ManagedSceneObject::OnPositionChanged(Object^ value, PropertyChangedEventArgs^ args)
	{
		// Update position property via property so cached value is also updated
		Position = static_cast<Vector3^>(value);
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
		RMesh* mesh = RResourceManager::Instance().FindResource<RMesh>(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer()));
		if (mesh)
			GetMeshObject()->SetMesh(mesh);
	}

	List<ManagedMaterial^>^ ManagedMeshObject::Materials::get()
	{
		List<ManagedMaterial^>^ MaterialList = gcnew List<ManagedMaterial^>();

		RSMeshObject* MeshObject = GetMeshObject();
		if (MeshObject)
		{
			int NumElements = MeshObject->GetMeshElementCount();
			RMesh* Mesh = MeshObject->GetMesh();
			if (Mesh)
			{
				for (int i = 0; i < NumElements; i++)
				{
					RMaterial* Material = MeshObject->GetMaterial(i);
					auto MeshElement = Mesh->GetMeshElements()[i];

					MaterialList->Add(gcnew ManagedMaterial(Material, MeshElement.GetName().c_str()));
				}
			}
		}

		return MaterialList;
	}

	String^ ManagedMeshObject::VertexComponents::get()
	{
		RSMeshObject* MeshObject = GetMeshObject();

		if (MeshObject)
		{
			RMesh* Mesh = MeshObject->GetMesh();
			int NumElements = MeshObject->GetMeshElementCount();

			if (Mesh && NumElements > 0)
			{
				auto MeshElement = Mesh->GetMeshElements()[0];
				int VertexComponentMask = MeshElement.GetVertexComponentMask();
				string VertexComponentsString = RVertexDeclaration::GetVertexComponentsString(VertexComponentMask);

				return gcnew String(VertexComponentsString.c_str());
			}
		}

		return gcnew String("");
	}

	Vector3^ ManagedSceneObject::Position::get()
	{
		float x, y, z;
		GetObjectPositionInFloat3(m_SceneObject, x, y, z);
		Vector3^ value = gcnew Vector3(x, y, z);

		// Store cached value
		CachedPosition = value;
		return %CachedPosition;
	}

	void ManagedSceneObject::Position::set(Vector3^ value)
	{
		CachedPosition = value;
		SetObjectPositionInFloat3(m_SceneObject, value->x, value->y, value->z);
	}

	System::String^ ManagedSceneObject::Rotation::get()
	{
		float x, y, z;
		GetObjectRotationInFloat3(m_SceneObject, x, y, z);
		return TypeUtils::Float3ToString(x, y, z);
	}

	void ManagedSceneObject::Rotation::set(String^ value)
	{
		float x, y, z;
		TypeUtils::StringToFloat3(value, x, y, z);
		SetObjectRotationInFloat3(m_SceneObject, x, y, z);
	}

	System::String^ ManagedSceneObject::Scale::get()
	{
		float x, y, z;
		GetObjectScaleInFloat3(m_SceneObject, x, y, z);
		return TypeUtils::Float3ToString(x, y, z);
	}

	void ManagedSceneObject::Scale::set(String^ value)
	{
		float x, y, z;
		TypeUtils::StringToFloat3(value, x, y, z);
		SetObjectScaleInFloat3(m_SceneObject, x, y, z);
	}

	System::String^ ManagedSceneObject::Script::get()
	{
		if (m_SceneObject)
		{
			const std::string& ScriptName = m_SceneObject->GetScript();
			return gcnew String(ScriptName.c_str());
		}

		return gcnew String("");
	}

	void ManagedSceneObject::Script::set(String^ value)
	{
		if (m_SceneObject)
		{
			IntPtr AnsiStringPtr = Marshal::StringToHGlobalAnsi(value);
			const char* ScriptName = static_cast<const char*>(AnsiStringPtr.ToPointer());
			m_SceneObject->SetScript(ScriptName);
		}
	}

}
