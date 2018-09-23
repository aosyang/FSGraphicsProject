//=============================================================================
// Vector3.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "TypeUtils.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Globalization;

namespace ManagedEngineWrapper
{
	/// The managed wrapper class of RVec3
	[TypeConverter("ManagedEngineWrapper.Vector3Converter")]		// Specify a type converter to convert between vector3 and string
	public ref class Vector3
		: INotifyPropertyChanged									// Provides PropertyChangedEventHandler
	{
	public:
		float x, y, z;

		Vector3() {}

		Vector3(float _x, float _y, float _z)
			: x(_x), y(_y), z(_z)
		{}

		Vector3(const Vector3% cpy)
		{
			x = cpy.x;
			y = cpy.y;
			z = cpy.z;
		}

		Vector3 operator=(const Vector3^ rhs)
		{
			x = rhs->x;
			y = rhs->y;
			z = rhs->z;

			return *this;
		}

		property float X
		{
			float get()				{ return x; }
			void set(float value)	{ x = value; OnPropertyChanged("X"); }
		}

		property float Y
		{
			float get()				{ return y; }
			void set(float value)	{ y = value; OnPropertyChanged("Y"); }
		}

		property float Z
		{
			float get()				{ return z; }
			void set(float value)	{ z = value; OnPropertyChanged("Z"); }
		}

		virtual event PropertyChangedEventHandler^ PropertyChanged;

	protected:
		virtual void OnPropertyChanged(String^ PropertyName)
		{
			PropertyChanged(this, gcnew PropertyChangedEventArgs(PropertyName));
		}
	};

	public ref class Vector3Converter : ExpandableObjectConverter
	{
	public:
		bool CanConvertTo(ITypeDescriptorContext^ context, Type^ destinationType) override
		{
			if (destinationType == Vector3::typeid)
			{
				return true;
			}

			return ExpandableObjectConverter::CanConvertTo(context, destinationType);
		}

		Object^ ConvertTo(ITypeDescriptorContext^ context, CultureInfo^ culture, Object^ value, Type^ destinationType) override
		{
			if (destinationType == String::typeid &&
				value->GetType() == Vector3::typeid)
			{
				Vector3^ vec3 = (Vector3^)value;

				return TypeUtils::Float3ToString(vec3->x, vec3->y, vec3->z);
			}

			return ExpandableObjectConverter::ConvertTo(context, culture, value, destinationType);
		}

		bool CanConvertFrom(ITypeDescriptorContext^ context, Type^ sourceType) override
		{
			if (sourceType == String::typeid)
			{
				return true;
			}

			return ExpandableObjectConverter::CanConvertFrom(context, sourceType);
		}

		Object^ ConvertFrom(ITypeDescriptorContext^ context, CultureInfo^ culture, Object^ value) override
		{
			if (value->GetType() == String::typeid)
			{
				String^ str = (String^)value;
				float x, y, z;
				TypeUtils::StringToFloat3(str, x, y, z);

				return gcnew Vector3(x, y, z);
			}

			return ExpandableObjectConverter::ConvertFrom(context, culture, value);
		}
	};
}