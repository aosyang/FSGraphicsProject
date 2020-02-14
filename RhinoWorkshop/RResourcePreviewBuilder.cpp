//=============================================================================
// RResourcePreviewBuilder.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "RResourcePreviewBuilder.h"

void RResourcePreviewBuilder::BuildPreviewForAllResources()
{
	std::vector<RResourceBase*> ResourceCollection = RResourceManager::Instance().EnumerateAllResources();
	for (auto Resource : ResourceCollection)
	{
		if (Resource->CanCastTo<RTexture>())
		{
			continue;
		}

		RTexture* PreviewTexture = GeneratePreviewTexture(Resource, 512, 512);
		if (PreviewTexture)
		{
			ResourcePreviews[Resource] = PreviewTexture;
		}
	}
}

void RResourcePreviewBuilder::BuildPreviewForResource(RResourceBase* Resource)
{
	if (Resource->CanCastTo<RTexture>())
	{
		return;
	}

	auto Iter = ResourcePreviews.find(Resource);
	if (Iter != ResourcePreviews.end())
	{
		// Destroy the wrapper texture resource here
	}

	RTexture* PreviewTexture = GeneratePreviewTexture(Resource, 512, 512);
	if (PreviewTexture)
	{
		ResourcePreviews[Resource] = PreviewTexture;
	}
}

RTexture* RResourcePreviewBuilder::FindPreviewTexture(RResourceBase* Resource) const
{
	auto Iter = ResourcePreviews.find(Resource);
	if (Iter != ResourcePreviews.end())
	{
		return Iter->second;
	}

	return nullptr;
}

RTexture* RResourcePreviewBuilder::GeneratePreviewTexture(RResourceBase* Resource, int Width, int Height)
{
	if (Resource)
	{
		ID3D11Texture2D*			RenderTargetBuffer;
		ID3D11RenderTargetView*		RenderTargetView;
		ID3D11Texture2D*			RenderTargetDepthBuffer;
		ID3D11DepthStencilView*		RenderTargetDepthView;

		ID3D11ShaderResourceView*	RenderTargetSRV;

		D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
		renderTargetTextureDesc.Width = Width;
		renderTargetTextureDesc.Height = Height;
		renderTargetTextureDesc.MipLevels = 1;
		renderTargetTextureDesc.ArraySize = 1;
		renderTargetTextureDesc.Format = GRenderer.UsingGammaCorrection() ? DXGI_FORMAT_B8G8R8X8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		renderTargetTextureDesc.SampleDesc.Count = 1;
		renderTargetTextureDesc.SampleDesc.Quality = 0;
		renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		renderTargetTextureDesc.CPUAccessFlags = 0;
		renderTargetTextureDesc.MiscFlags = 0;

		GRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &RenderTargetBuffer);
		GRenderer.D3DDevice()->CreateRenderTargetView(RenderTargetBuffer, 0, &RenderTargetView);

		D3D11_SHADER_RESOURCE_VIEW_DESC rtsrvDesc;
		rtsrvDesc.Format = renderTargetTextureDesc.Format;
		rtsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		rtsrvDesc.Texture2D.MostDetailedMip = 0;
		rtsrvDesc.Texture2D.MipLevels = 1;

		GRenderer.D3DDevice()->CreateShaderResourceView(RenderTargetBuffer, &rtsrvDesc, &RenderTargetSRV);

		renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		renderTargetTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		renderTargetTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		renderTargetTextureDesc.CPUAccessFlags = 0;

		GRenderer.D3DDevice()->CreateTexture2D(&renderTargetTextureDesc, 0, &RenderTargetDepthBuffer);
		GRenderer.D3DDevice()->CreateDepthStencilView(RenderTargetDepthBuffer, 0, &RenderTargetDepthView);

		RScene PreviewScene;
		RScene* LastActiveScene = GRenderer.GetActiveScene();

		GRenderer.SetActiveScene(&PreviewScene);

		GRenderer.SetSamplerState(0, SamplerState_Texture);
		GRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);
		GRenderer.SetRenderTargets(1, &RenderTargetView, RenderTargetDepthView);

		D3D11_VIEWPORT vp = { 0.0f, 0.0f, (FLOAT)Width, (FLOAT)Height, 0.0f, 1.0f };
		GRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);

		GRenderer.Clear();

		RSceneObject* MeshObject = CreatePreviewSceneObject(PreviewScene, Resource);
		RCamera* Camera = PreviewScene.CreateSceneObjectOfType<RCamera>();

		float AspectRatio = (float)Width / (float)Height;
		Camera->SetupView(45.0f, AspectRatio, 1.0f, 10000.0f);

		PreviewScene.UpdateScene(0.0f);

		RAabb aabb = MeshObject->GetAabb();
		RVec3 center = (aabb.pMin + aabb.pMax) * 0.5f;
		float radius = (aabb.pMax - center).Magnitude();

		// Offset from center of the bound to object's origin
		RVec3 Offset = MeshObject->GetWorldPosition() - center;

		// Fit the object's aabb into vertical FOV of the camera
		float HalfFov = Camera->GetFOV() * 0.5f;

		// Make material meshes bigger
		float FovFactor = Resource->CanCastTo<RMaterial>() ? 1.4f : 0.8f;
		float Distance = radius / tanf(RMath::DegreeToRadian(HalfFov * FovFactor));

		RVec3 CameraDir = RVec3(1, 1, -2).GetNormalized();

		Camera->SetPosition(center + CameraDir * Distance);
		Camera->LookAt(center);

		auto& cbGlobal = RConstantBuffers::cbGlobal.Data;
		RConstantBuffers::cbGlobal.ClearData();

		cbGlobal.UseGammaCorrection = GRenderer.UsingGammaCorrection();

		RConstantBuffers::cbGlobal.UpdateBufferData();
		RConstantBuffers::cbGlobal.BindBuffer();

		RMatrix4 viewMatrix = Camera->GetViewMatrix();
		RMatrix4 projMatrix = Camera->GetProjectionMatrix();

		// Update scene constant buffer
		auto& cbScene = RConstantBuffers::cbScene.Data;
		RConstantBuffers::cbScene.ClearData();

		cbScene.viewMatrix = viewMatrix;
		cbScene.projMatrix = projMatrix;
		cbScene.viewProjMatrix = viewMatrix * projMatrix;
		cbScene.cameraPos = Camera->GetWorldPosition();

		RConstantBuffers::cbScene.UpdateBufferData();
		RConstantBuffers::cbScene.BindBuffer();

		RVec3 SunDirection = RVec3(-1, 1, -1).GetNormalized();

		RConstantBuffers::cbLight.ClearData();
		RConstantBuffers::cbLight.Data.HighHemisphereAmbientColor = RVec4(0.4f, 0.4f, 0.4f, 1.0f);
		RConstantBuffers::cbLight.Data.LowHemisphereAmbientColor = RVec4(0.1f, 0.1f, 0.1f, 1.0f);
		RConstantBuffers::cbLight.Data.DirectionalLightCount = 1;
		RConstantBuffers::cbLight.Data.DirectionalLight->Color = RVec4(1.0f, 1.0f, 0.8f, 1.0f);
		RConstantBuffers::cbLight.Data.DirectionalLight->Direction = RVec4(SunDirection, 0.0f);
		RConstantBuffers::cbLight.UpdateBufferData();
		RConstantBuffers::cbLight.BindBuffer();

		RConstantBuffers::cbMaterial.ClearData();
		RConstantBuffers::cbMaterial.Data.GlobalOpacity = 1.0f;
		RConstantBuffers::cbMaterial.Data.SpecularColorAndPower = RVec4(1, 1, 1, 32);
		RConstantBuffers::cbMaterial.UpdateBufferData();
		RConstantBuffers::cbMaterial.BindBuffer();

		if (IsSkinnedMesh(Resource))
		{
			RConstantBuffers::cbBoneMatrices.ClearData();
			for (int i = 0; i < MAX_BONE_COUNT; i++)
			{
				RConstantBuffers::cbBoneMatrices.Data.boneMatrix[i] = RMatrix4::IDENTITY;
			}
			RConstantBuffers::cbBoneMatrices.UpdateBufferData();
			RConstantBuffers::cbBoneMatrices.BindBuffer();
		}

		PreviewScene.Render();
		GRenderer.Present(false);

		GRenderer.SetActiveScene(LastActiveScene);

		// Unbind preview scene render targets
		GRenderer.SetRenderTargets();

		RTexture* PreviewTexture = RResourceManager::Instance().WrapShaderResourceViewInTexture(RenderTargetSRV, true);

		PreviewScene.Release();
		SAFE_RELEASE(RenderTargetBuffer);
		SAFE_RELEASE(RenderTargetView);
		SAFE_RELEASE(RenderTargetDepthBuffer);
		SAFE_RELEASE(RenderTargetDepthView);

		return PreviewTexture;
	}

	return nullptr;
}

bool RResourcePreviewBuilder::IsSkinnedMesh(RResourceBase* Resource)
{
	RMesh* Mesh = Resource->CastTo<RMesh>();
	if (Mesh)
	{
		return Mesh->HasAnySkinnedMeshElements();
	}

	return false;
}

RSceneObject* RResourcePreviewBuilder::CreatePreviewSceneObject(RScene& Scene, RResourceBase* Resource)
{
	if (RMesh* Mesh = Resource->CastTo<RMesh>())
	{
		return Scene.CreateMeshObject(Mesh);
	}
	else if (RMaterial* Material = Resource->CastTo<RMaterial>())
	{
		RMesh* MaterialPreviewMesh = RResourceManager::Instance().FindResource<RMesh>("/sphere.fbx", true);
		RSMeshObject* MeshObject = Scene.CreateMeshObject(MaterialPreviewMesh);

		std::vector<RMaterial*> Materials;
		Materials.push_back(Material);
		MeshObject->SetMaterials(Materials);

		return MeshObject;
	}

	return nullptr;
}
