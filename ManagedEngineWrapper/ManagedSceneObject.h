#pragma once

#include "ManagedMaterial.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;
using namespace System::Globalization;
using namespace System::Runtime::InteropServices;
using namespace ManagedInterface;

class RSceneObject;

namespace ManagedEngineWrapper
{
	public ref class ManagedSceneObject : public IManagedSceneObject
	{
	protected:
		RSceneObject*	m_SceneObject;
	public:
		ManagedSceneObject(RSceneObject* obj);

		virtual bool IsValid();

		/// Access the name of scene object
		[Category("Scene Object")]
		property String^ Name
		{
			String^ get()           { return gcnew String(m_SceneObject->GetName().c_str()); }
			void set(String^ value) { m_SceneObject->SetName(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer())); }
		};

		/// Access the position of scene object
		[Category("Scene Object")]
		property String^ Position
		{
			String^ get();
			void set(String^ value);
		};

		/// Access the rotation of scene object
		[Category("Scene Object")]
		property String^ Rotation
		{
			String^ get();
			void set(String^ value);
		};

		/// Access the scale of scene object
		[Category("Scene Object")]
		property String^ Scale
		{
			String^ get();
			void set(String^ value);
		};

		[Category("Scene Object")]
		property String^ Script
		{
			String^ get()			{ return gcnew String(m_SceneObject->GetScript().c_str()); }
			void set(String^ value)	{ m_SceneObject->SetScript(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer())); }
		}

	private:
		String^ Float3ToString(float x, float y, float z);
		void StringToFloat3(String^ str, float& x, float &y, float &z);
	};

	public ref class ManagedMeshObject : ManagedSceneObject
	{
	private:
		RSMeshObject* GetMeshObject() { return static_cast<RSMeshObject*>(m_SceneObject); }
	public:
		ManagedMeshObject(RSceneObject* obj);

		[Category("Mesh Object")]
		[DisplayName("Mesh Asset")]
		property String^ Asset
		{
			String^ get();
			void set(String^ value);
		};

		[Category("Mesh Object")]
		[DisplayName("Vertex Components")]
		property String^ VertexComponents
		{
			String^ get();
		}

		[Category("Mesh Object")]
		property List<ManagedMaterial^>^ Materials
		{
			List<ManagedMaterial^>^ get();
		}

		[Category("Mesh Object")]
		[DisplayName("Submesh Count")]
		property int SubMeshCount
		{
			int get()				{ return GetMeshObject()->GetMeshElementCount(); }
		}

		//property String^ Shader
		//{
		//	String^ get() { return gcnew String(RShaderManager::Instance().GetShaderName(GetMeshObject()->GetMaterial(0)->Shader).c_str()); }
		//}

		//[TypeConverter(ExpandableObjectConverter::typeid)]
		//property ManagedMaterial^ material
		//{
		//	ManagedMaterial^ get() { return gcnew ManagedMaterial(GetMeshObject()->GetMaterial(0)); }
		//}

		//[TypeConverter(ManagedMaterialCollectionConverter::typeid)]
		//property ManagedMaterialCollection^ MaterialCollection
		//{
		//	ManagedMaterialCollection^ get() { return gcnew ManagedMaterialCollection(GetMeshObject()); }
		//}
	};
}
