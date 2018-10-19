//=============================================================================
// ManagedEngineWrapper.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// This is the main DLL file.
//=============================================================================

#include "stdafx.h"

#include "ManagedEngineWrapper.h"
#include "EditorApp.h"
#include "ManagedSceneObject.h"

#pragma comment(lib, "User32.lib")

#include <direct.h>

using namespace System::Runtime::InteropServices;

namespace ManagedEngineWrapper
{
	RKeyStateModifier g_KeyStateModifier;

	const char* ManagedStringRefToConstCharPtr(String^ str)
	{
		IntPtr pNativeStr = Marshal::StringToHGlobalAnsi(str);
		return static_cast<const char*>(pNativeStr.ToPointer());
	}

	RhinoEngineWrapper::RhinoEngineWrapper()
	{
		m_IsInitialized = false;
		m_Application = new EditorApp();
		GEngine.BindApp(m_Application);
	}

	RhinoEngineWrapper::~RhinoEngineWrapper()
	{
		this->!RhinoEngineWrapper();
	}

	RhinoEngineWrapper::!RhinoEngineWrapper()
	{
	}

	bool RhinoEngineWrapper::Initialize(IntPtr hWnd)
	{
		RECT rect;
		GetClientRect((HWND)hWnd.ToPointer(), &rect);

		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		m_IsInitialized = GEngine.Initialize((HWND)hWnd.ToPointer(), width, height);
		return m_IsInitialized;
	}

	void RhinoEngineWrapper::RunOneFrame()
	{
		if (m_IsInitialized)
		{
			GEngine.RunOneFrame(true);
		}
	}

	void RhinoEngineWrapper::Shutdown()
	{
		if (m_IsInitialized)
		{
			GEngine.Shutdown();

			delete m_Application;
		}
	}

	void RhinoEngineWrapper::Resize(int width, int height)
	{
		if (m_IsInitialized)
			GEngine.ResizeClientWindow(width, height);
	}

	List<String^>^ RhinoEngineWrapper::GetMeshNameList()
	{
		List<String^>^ list = gcnew List<String^>();

		const vector<RMesh*>& meshList = RResourceManager::Instance().GetMeshResources();
		for (auto Iter : meshList)
		{
			list->Add(gcnew String(Iter->GetPath().data()));
		}

		return list;
	}

	bool RhinoEngineWrapper::IsMeshAssetReady(String^ MeshName)
	{
		RMesh* MeshAsset = RResourceManager::Instance().FindMesh(ManagedStringRefToConstCharPtr(MeshName));
		if (MeshAsset)
		{
			return MeshAsset->IsLoaded() && MeshAsset->AreReferencedResourcesLoaded();
		}

		return false;
	}

	Bitmap^ RhinoEngineWrapper::GenerateMeshThumbnailBitmap(String^ MeshName, int Width, int Height)
	{
		RMesh* MeshAsset = RResourceManager::Instance().FindMesh(ManagedStringRefToConstCharPtr(MeshName));
		return RenderThumbnailForMesh(MeshAsset, Width, Height);
	}

	void RhinoEngineWrapper::AddMeshObjectToScene(String^ MeshAssetPath)
	{
		const char* NativePath = ManagedStringRefToConstCharPtr(MeshAssetPath);
		m_Application->AddMeshObjectToScene(NativePath);
		UpdateSceneObjectsList();
	}

	void RhinoEngineWrapper::ReplaceMeshAssetForSelection(String^ MeshAssetPath)
	{
		RSMeshObject* meshObj = static_cast<RSMeshObject*>(m_Application->GetSelection());
		if (meshObj)
		{
			const char* NativePath = ManagedStringRefToConstCharPtr(MeshAssetPath);
			RMesh* MeshResource = RResourceManager::Instance().FindMesh(NativePath);
			if (MeshResource)
			{
				meshObj->SetMesh(MeshResource);
			}
		}
	}

	List<IManagedSceneObject^>^ RhinoEngineWrapper::GetSceneObjectsList()
	{
		return SceneObjectsList;
	}

	void RhinoEngineWrapper::OnKeyDown(int keycode)
	{
		g_KeyStateModifier.NotifyKeyDownStateChanged(keycode, true);
	}

	void RhinoEngineWrapper::OnKeyUp(int keycode)
	{
		g_KeyStateModifier.NotifyKeyDownStateChanged(keycode, false);
	}

	void RhinoEngineWrapper::SetInputEnabled(bool bEnabled)
	{
		m_Application->SetInputEnabled(bEnabled);
	}

	void RhinoEngineWrapper::RunScreenToCameraRayPicking(float x, float y)
	{
		//m_Application->RunScreenToCameraRayPicking(x, y);
	}

	IManagedSceneObject^ RhinoEngineWrapper::GetSelection()
	{
		RSceneObject* sel = m_Application->GetSelection();
		if (sel)
		{
			if (!SceneObjectsList)
			{
				UpdateSceneObjectsList();
			}

			for (int i = 0; i < SceneObjectsList->Count; i++)
			{
				auto SceneObject = safe_cast<ManagedSceneObject^>(SceneObjectsList[i]);
				if (SceneObject->GetRawSceneObjectPtr() == sel)
				{
					return SceneObject;
				}
			}
		}

		return gcnew ManagedEngineWrapper::ManagedSceneObject(sel);
	}

	void RhinoEngineWrapper::SetSelection(IManagedSceneObject^ SelectedSceneObject)
	{
		if (SelectedSceneObject != nullptr)
		{
			ManagedSceneObject^ SceneObject = safe_cast<ManagedSceneObject^>(SelectedSceneObject);
			if (SceneObject != nullptr)
			{
				m_Application->SetSelection(SceneObject->GetRawSceneObjectPtr());
				return;
			}
		}

		m_Application->SetSelection(nullptr);
	}

	bool RhinoEngineWrapper::DeleteSelection()
	{
		if (m_Application->DeleteSelection())
		{
			UpdateSceneObjectsList();
			return true;
		}

		return false;
	}

	void RhinoEngineWrapper::LoadScene(String^ filename)
	{
		m_Application->LoadScene(ManagedStringRefToConstCharPtr(filename));
		UpdateSceneObjectsList();
	}

	void RhinoEngineWrapper::SaveScene(String^ filename)
	{
		m_Application->SaveScene(ManagedStringRefToConstCharPtr(filename));
	}

	void RhinoEngineWrapper::SaveMeshMaterialFromSelection()
	{
		m_Application->SaveMeshMaterialFromSelection();
	}

	void RhinoEngineWrapper::ExportAllAnimationsToBinaryFiles()
	{
		m_Application->ExportAllAnimationsToBinaryFiles();
	}

	void RhinoEngineWrapper::OnAsyncResourceLoaded(const char* ResourceName)
	{
		if (AsyncResourceLoadedCallback != nullptr)
		{
			// Convert resource name and call managed delegate
			String^ ManagedResourceName = gcnew String(ResourceName);
			AsyncResourceLoadedCallback(ManagedResourceName);
		}
	}

	void RhinoEngineWrapper::SetAsyncResourceLoadedHandler(ManagedInterface::AsyncResourceLoadedHandler^ AsyncResourceLoaded)
	{
		// Save the managed delegate handle
		AsyncResourceLoadedCallback = AsyncResourceLoaded;

		// Make a new delegate and bind to async resource loaded event
		AsyncResourceLoadedWrapperDelegate = gcnew AsyncResourceLoadedWrapper(this, &RhinoEngineWrapper::OnAsyncResourceLoaded);
		IntPtr pFunc = Marshal::GetFunctionPointerForDelegate(AsyncResourceLoadedWrapperDelegate);
		m_Application->SetOnAsyncResourceLoadedCallback(static_cast<NativeAsyncResourceLoadedCallback>(pFunc.ToPointer()));
	}

	void RhinoEngineWrapper::UpdateSceneObjectsList()
	{
		SceneObjectsList = gcnew List<IManagedSceneObject ^>();

		for (auto SceneObject : m_Application->GetSceneObjects())
		{
			if (SceneObject->IsType<RSMeshObject>())
			{
				SceneObjectsList->Add(gcnew ManagedMeshObject(SceneObject));
			}
			else
			{
				SceneObjectsList->Add(gcnew ManagedSceneObject(SceneObject));
			}
		}
	}

	Bitmap^ RhinoEngineWrapper::RenderThumbnailForMesh(RMesh* MeshAsset, int Width, int Height)
	{
		// TODO: Need to reuse some engine code here

		if (MeshAsset)
		{
			ID3D11Texture2D*			RenderTargetBuffer;
			ID3D11RenderTargetView*		RenderTargetView;
			ID3D11ShaderResourceView*	RenderTargetSRV;
			ID3D11Texture2D*			RenderTargetDepthBuffer;
			ID3D11DepthStencilView*		RenderTargetDepthView;
			ID3D11Texture2D*			ReadbackBuffer;

			D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
			renderTargetTextureDesc.Width = Width;
			renderTargetTextureDesc.Height = Height;
			renderTargetTextureDesc.MipLevels = 1;
			renderTargetTextureDesc.ArraySize = 1;
			renderTargetTextureDesc.Format = GRenderer.UsingGammaCorrection() ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
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

			// D3D doesn't allow fetching pixel from a backbuffer directly. Need a new buffer to copy from backbuffer for CPU reading.
			D3D11_TEXTURE2D_DESC ReadbackTextureDesc;
			ReadbackTextureDesc.Width = Width;
			ReadbackTextureDesc.Height = Height;
			ReadbackTextureDesc.MipLevels = 1;
			ReadbackTextureDesc.ArraySize = 1;
			ReadbackTextureDesc.Format = GRenderer.UsingGammaCorrection() ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
			ReadbackTextureDesc.SampleDesc.Count = 1;
			ReadbackTextureDesc.SampleDesc.Quality = 0;
			ReadbackTextureDesc.Usage = D3D11_USAGE_STAGING;
			ReadbackTextureDesc.BindFlags = 0;
			ReadbackTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			ReadbackTextureDesc.MiscFlags = 0;

			GRenderer.D3DDevice()->CreateTexture2D(&ReadbackTextureDesc, 0, &ReadbackBuffer);


			RScene PreviewScene;
			RScene* LastActiveScene = GRenderer.GetActiveScene();

			GRenderer.SetActiveScene(&PreviewScene);

			GRenderer.SetSamplerState(0, SamplerState_Texture);
			GRenderer.SetSamplerState(2, SamplerState_ShadowDepthComparison);
			GRenderer.SetRenderTargets(1, &RenderTargetView, RenderTargetDepthView);

			D3D11_VIEWPORT vp = { 0.0f, 0.0f, (FLOAT)Width, (FLOAT)Height, 0.0f, 1.0f };
			GRenderer.D3DImmediateContext()->RSSetViewports(1, &vp);

			GRenderer.Clear();

			RSMeshObject* MeshObject = PreviewScene.CreateMeshObject(MeshAsset);
			RCamera* Camera = PreviewScene.CreateSceneObjectOfType<RCamera>();

			float AspectRatio = (float)Width / (float)Height;
			Camera->SetupView(45.0f, AspectRatio, 1.0f, 10000.0f);

			RAabb aabb = MeshObject->GetAabb();
			RVec3 center = (aabb.pMin + aabb.pMax) * 0.5f;
			float radius = (aabb.pMax - center).Magnitude();

			// Offset from center of the bound to object's origin
			RVec3 Offset = MeshObject->GetWorldPosition() - center;

			// Fit the object's aabb into vertical FOV of the camera
			float HalfFov = Camera->GetFOV() * 0.5f;
			float Distance = radius / tanf(RMath::DegreeToRadian(HalfFov * 0.8f));

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

			RConstantBuffers::cbLight.ClearData();
			RConstantBuffers::cbLight.Data.HighHemisphereAmbientColor = RVec4(0.4f, 0.4f, 0.4f, 1.0f);
			RConstantBuffers::cbLight.Data.LowHemisphereAmbientColor = RVec4(0.1f, 0.1f, 0.1f, 1.0f);
			RConstantBuffers::cbLight.UpdateBufferData();
			RConstantBuffers::cbLight.BindBuffer();

			RConstantBuffers::cbMaterial.ClearData();
			RConstantBuffers::cbMaterial.UpdateBufferData();
			RConstantBuffers::cbMaterial.BindBuffer();

			PreviewScene.Render();

			GRenderer.Present();

			GRenderer.SetActiveScene(LastActiveScene);

			// Unbind preview scene render targets
			GRenderer.SetRenderTargets();

			D3D11_BOX BoxArea;
			BoxArea.left = 0;
			BoxArea.top = 0;
			BoxArea.right = Width;
			BoxArea.bottom = Height;
			BoxArea.front = 0;
			BoxArea.back = 1;
			GRenderer.D3DImmediateContext()->CopySubresourceRegion(ReadbackBuffer, 0, 0, 0, 0, RenderTargetBuffer, 0, &BoxArea);

			D3D11_MAPPED_SUBRESOURCE TextureResource;
			GRenderer.D3DImmediateContext()->Map(ReadbackBuffer, 0, D3D11_MAP_READ, 0, &TextureResource);

			Bitmap^ Image = gcnew Bitmap(Width, Height);

			for (int y = 0; y < Height; y++)
			{
				for (int x = 0; x < Width; x++)
				{
					int Index = y * Width + x;
					int ColorValue = ((int*)TextureResource.pData)[Index];
					BYTE R = ColorValue & 0xFF;
					BYTE G = (ColorValue & 0xFF00) >> 8;
					BYTE B = (ColorValue & 0xFF0000) >> 16;
					Image->SetPixel(x, y, Color::FromArgb(255, R, G, B));
				}
			}
			GRenderer.D3DImmediateContext()->Unmap(ReadbackBuffer, 0);

			SAFE_RELEASE(RenderTargetBuffer);
			SAFE_RELEASE(RenderTargetView);
			SAFE_RELEASE(RenderTargetSRV);
			SAFE_RELEASE(RenderTargetDepthBuffer);
			SAFE_RELEASE(RenderTargetDepthView);
			SAFE_RELEASE(ReadbackBuffer);

			return Image;
		}

		return nullptr;
	}
}
