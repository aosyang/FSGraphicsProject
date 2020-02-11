//=============================================================================
// ManagedMaterial.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "stdafx.h"

#include "ManagedMaterial.h"

namespace ManagedEngineWrapper
{

	ManagedMaterial::ManagedMaterial(RMaterial* mat, const char* elemName)
		: material(mat)
		, meshElementName(gcnew String(elemName))
		, ShaderWrapper(mat->GetShader())
	{

	}

	ManagedMaterialCollection::ManagedMaterialCollection(RSMeshObject* MeshObject)
	{
		if (MeshObject)
		{
			RMesh* Mesh = MeshObject->GetMesh();
			if (Mesh)
			{
				int NumElements = MeshObject->GetMeshElementCount();
				for (int i = 0; i < NumElements; i++)
				{
					RMaterial* Material = MeshObject->GetMaterial(i);
					auto MeshElement = Mesh->GetMeshElements()[i];

					materials.Add(gcnew ManagedMaterial(Material, MeshElement.GetName().c_str()));
				}
			}
		}
	}
}
