#include "stdafx.h"
#include "ManagedSceneObject.h"

namespace EngineManagedWrapper {

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
		String^ str = String::Format(L"{0}, {1}, {2}", vec.x, vec.y, vec.z);
		return str;
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

}