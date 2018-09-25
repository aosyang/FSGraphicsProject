//=============================================================================
// ManagedMaterial.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "ManagedShader.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;
using namespace System::Globalization;
using namespace System::Runtime::InteropServices;

namespace ManagedEngineWrapper
{
	/// The managed wrapper class of RMaterial
	public ref class ManagedMaterial
	{
		RMaterial* material;
		String^ meshElementName;
		ManagedShader ShaderWrapper;
	public:
		ManagedMaterial(RMaterial* mat, const char* elemName);

		property int TextureCount
		{
			int get()				{ return material->TextureNum; }
			void set(int value)		{ material->TextureNum = value; }
		}

		property ManagedShader^ Shader
		{
			ManagedShader^ get()
			{
				ShaderWrapper.SetInstance(material->Shader);
				return %ShaderWrapper;
			}

			void set(ManagedShader^ value)
			{
				ShaderWrapper = value;
				material->Shader = ShaderWrapper.GetInstance();
			}
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
		ManagedMaterialCollection(RSMeshObject* MeshObject);

		int GetCount() { return materials.Count; }
		ManagedMaterial^ GetMaterial(int i) { return materials[i]; }
	};

	/// Converter from material collections to PropertyGrid expandable attributes
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
}
