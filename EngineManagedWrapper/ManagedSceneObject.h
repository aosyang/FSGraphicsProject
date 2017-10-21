#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;
using namespace System::Globalization;

class RSceneObject;

namespace EngineManagedWrapper {

	public ref class ManagedMaterial
	{
		RMaterial* material;
		String^ meshElementName;
	public:
		ManagedMaterial(RMaterial* mat, const char* elemName);

		property int TextureCount
		{
			int get()				{ return material->TextureNum; }
			void set(int value)		{ material->TextureNum = value; }
		}

		property String^ Shader
		{
			String^ get()			{ return material->Shader ? gcnew String(material->Shader->GetName().c_str()) : gcnew String(""); }
			void set(String^ value)	{ material->Shader = RShaderManager::Instance().GetShaderResource(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer())); }
		}

#define DECLARE_TEXTURE_PROPERTY(n) \
		[DisplayName("Texture Slot "#n)] \
		property String^ Texture##n \
		{ \
			String^ get()			{ return material->Textures[n] ? gcnew String(material->Textures[n]->GetPath().c_str()) : gcnew String(""); } \
			void set(String^ value)	{ material->Textures[n] = value != "" ? RResourceManager::Instance().FindTexture(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer())) : nullptr; } \
		}

		DECLARE_TEXTURE_PROPERTY(0)
		DECLARE_TEXTURE_PROPERTY(1)
		DECLARE_TEXTURE_PROPERTY(2)
		DECLARE_TEXTURE_PROPERTY(3)
		DECLARE_TEXTURE_PROPERTY(4)
		DECLARE_TEXTURE_PROPERTY(5)
		DECLARE_TEXTURE_PROPERTY(6)
		DECLARE_TEXTURE_PROPERTY(7)

		String^ ToString() override
		{
			String^ str = meshElementName + ": ";
			if (material->Shader)
				str += gcnew String(material->Shader->GetName().c_str());

			return str;
		}
	};


	public ref class ManagedMaterialCollection
	{
		List<ManagedMaterial^> materials;
	public:
		ManagedMaterialCollection(RSMeshObject* obj);

		int GetCount() { return materials.Count; }
		ManagedMaterial^ GetMaterial(int i) { return materials[i]; }
	};

	public ref class ManagedMaterialCollectionConverter : ExpandableObjectConverter
	{
	public:
		bool CanConvertTo(ITypeDescriptorContext^ context, Type^ destinationType) override
		{
			if (destinationType == ManagedMaterial::typeid)
				return true;

			return ExpandableObjectConverter::CanConvertTo(context, destinationType);
		}

		Object^ ConvertTo(ITypeDescriptorContext^ context,
						  CultureInfo^ culture,
						  Object^ value,
						  Type^ destinationType) override
		{
			if (destinationType == String::typeid &&
				value->GetType() == ManagedMaterialCollection::typeid)
			{

				ManagedMaterialCollection^ mc = (ManagedMaterialCollection^)value;

				String^ str = gcnew String("");
				for (int i = 0; i < mc->GetCount(); i++)
				{
					str += mc->GetMaterial(i)->Shader + ";";
				}

				return str;
			}
			return ExpandableObjectConverter::ConvertTo(context, culture, value, destinationType);
		}
	};

	public ref class ManagedSceneObject
	{
	protected:
		RSceneObject*	m_SceneObject;
	public:
		ManagedSceneObject(RSceneObject* obj);

		bool IsValid();

		[Category("Scene Object")]
		property String^ Name
		{
			String^ get()           { return gcnew String(m_SceneObject->GetName().c_str()); }
			void set(String^ value) { m_SceneObject->SetName(static_cast<const char*>(Marshal::StringToHGlobalAnsi(value).ToPointer())); }
		};

		[Category("Scene Object")]
		property String^ Position
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
