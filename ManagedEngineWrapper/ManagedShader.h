//=============================================================================
// ManagedShader.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

using namespace System;
using namespace System::Collections::Generic;
using namespace System::ComponentModel;
using namespace System::Globalization;
using namespace System::Runtime::InteropServices;

namespace ManagedEngineWrapper
{
	[TypeConverter("ManagedEngineWrapper.ShaderConverter")]
	public ref class ManagedShader
	{
		RShader* Shader;

	public:
		ManagedShader(RShader* InShader)
			: Shader(InShader)
		{
		}

		ManagedShader^ operator=(ManagedShader^ rhs)
		{
			Shader = rhs->Shader;
			return this;
		}

		RShader* GetInstance()
		{
			return Shader;
		}

		void SetInstance(RShader* InShader)
		{
			Shader = InShader;
		}
	};

	// StringConverter provides GetStandardValues() method for a dropdown box
	public ref class ShaderConverter : StringConverter
	{
	public:
		virtual bool CanConvertTo(ITypeDescriptorContext^ context, Type^ destinationType) override
		{
			if (destinationType == ManagedShader::typeid)
			{
				return true;
			}

			return StringConverter::CanConvertTo(context, destinationType);
		}

		/// Convert object type to string
		virtual Object^ ConvertTo(ITypeDescriptorContext^ context, CultureInfo^ culture, Object^ value, Type^ destinationType) override
		{
			if (destinationType == String::typeid &&
				value->GetType() == ManagedShader::typeid)
			{
				RShader* NativeShader = safe_cast<ManagedShader^>(value)->GetInstance();

				return gcnew String(NativeShader->GetName().c_str());
			}

			return StringConverter::ConvertTo(context, culture, value, destinationType);
		}

		virtual bool CanConvertFrom(ITypeDescriptorContext^ context, Type^ sourceType) override
		{
			if (sourceType == String::typeid)
			{
				return true;
			}

			return StringConverter::CanConvertFrom(context, sourceType);
		}

		virtual Object^ ConvertFrom(ITypeDescriptorContext^ context, CultureInfo^ culture, Object^ value) override
		{
			if (value->GetType() == String::typeid)
			{
				String^ str = (String^)value;
				const char* AnsiShaderName = static_cast<const char*>(Marshal::StringToHGlobalAnsi(str).ToPointer());
				RShader* NativeShader = RShaderManager::Instance().GetShaderResource(AnsiShaderName);

				return gcnew ManagedShader(NativeShader);
			}

			return StringConverter::ConvertFrom(context, culture, value);
		}

		virtual bool GetStandardValuesSupported(ITypeDescriptorContext^ context) override { return true; }
		virtual bool GetStandardValuesExclusive(ITypeDescriptorContext^ context) override { return true; }

		virtual StandardValuesCollection^ GetStandardValues(ITypeDescriptorContext^ context) override
		{
			List<String^>^ list = gcnew List<String ^>();
			auto ShaderNameList = RShaderManager::Instance().EnumerateAllShaderNames();
			for (const auto& Name : ShaderNameList)
			{
				list->Add(gcnew String(Name.c_str()));
			}

			return gcnew StandardValuesCollection(list);
		}
	};
}
