#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

class RSceneObject;

namespace EngineManagedWrapper {

	public ref class ManagedSceneObject
	{
	protected:
		RSceneObject*	m_SceneObject;
	public:
		ManagedSceneObject(RSceneObject* obj);

		bool IsValid();

		property String^ Name
		{
			String^ get()           { return gcnew String(m_SceneObject->GetName().c_str()); }
			void set(String^ value) { m_SceneObject->SetName(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer())); }
		};

		property String^ Position
		{
			String^ get()           { return Vec3ToString(m_SceneObject->GetPosition()); }
			void set(String^ value) { m_SceneObject->SetPosition(StringToVec3(value)); }
		};

	private:
		String^ Vec3ToString(const RVec3& vec);
		RVec3 StringToVec3(String^ str);
		};

	public ref class ManagedMeshObject : ManagedSceneObject
	{
	private:
		RSMeshObject* GetMeshObject() { return static_cast<RSMeshObject*>(m_SceneObject); }
	public:
		ManagedMeshObject(RSceneObject* obj);

		property String^ Asset
		{
			String^ get()
			{
				RMesh* mesh = GetMeshObject()->GetMesh();
				if (mesh)
					return gcnew String(mesh->GetPath().c_str());
				else
					return gcnew String("");
			}

			void set(String^ value)
			{
				RMesh* mesh = RResourceManager::Instance().FindMesh(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer()));
				if (mesh)
					GetMeshObject()->SetMesh(mesh);
			}
		};
	};
}
