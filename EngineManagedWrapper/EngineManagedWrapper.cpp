// This is the main DLL file.

#include "stdafx.h"
#include "Rhino.h"

#include "EngineManagedWrapper.h"
#include "RSkybox.h"

#include "Skybox_PS.csh"
#include "Skybox_VS.csh"
#include "Default_PS.csh"
#include "Default_VS.csh"

#include "ConstBufferPS.h"
#include "ConstBufferVS.h"

#pragma comment(lib, "User32.lib")

#include <direct.h>

using namespace System::Runtime::InteropServices;

namespace EngineManagedWrapper
{
	class EditorApp : public IApp
	{
	private:
		RSkybox						m_Skybox;
		ID3D11Buffer*				m_cbPerObject;
		ID3D11Buffer*				m_cbScene;
		ID3D11SamplerState*			m_SamplerState;
		float						m_CamYaw, m_CamPitch;
		RSMeshObject				m_MeshObject;
		ID3D11InputLayout*			m_MeshInputLayout;
		RShader*					m_DefaultShader;
		RMatrix4					m_CameraMatrix;

	public:
		~EditorApp()
		{
			m_Skybox.Release();
			SAFE_RELEASE(m_cbPerObject);
			SAFE_RELEASE(m_cbScene);
			SAFE_RELEASE(m_SamplerState);
			SAFE_RELEASE(m_MeshInputLayout);

			RShaderManager::Instance().UnloadAllShaders();
			RResourceManager::Instance().Destroy();
		}

		bool Initialize()
		{
			RResourceManager::Instance().Initialize();

			D3D11_BUFFER_DESC cbPerObjectDesc;
			ZeroMemory(&cbPerObjectDesc, sizeof(cbPerObjectDesc));
			cbPerObjectDesc.ByteWidth = sizeof(SHADER_OBJECT_BUFFER);
			cbPerObjectDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbPerObjectDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbPerObjectDesc.Usage = D3D11_USAGE_DYNAMIC;

			RRenderer.D3DDevice()->CreateBuffer(&cbPerObjectDesc, NULL, &m_cbPerObject);

			D3D11_BUFFER_DESC cbSceneDesc;
			ZeroMemory(&cbSceneDesc, sizeof(cbSceneDesc));
			cbSceneDesc.ByteWidth = sizeof(SHADER_SCENE_BUFFER);
			cbSceneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			cbSceneDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			cbSceneDesc.Usage = D3D11_USAGE_DYNAMIC;

			RRenderer.D3DDevice()->CreateBuffer(&cbSceneDesc, NULL, &m_cbScene);

			RShaderManager::Instance().AddShader("Skybox", Skybox_PS, sizeof(Skybox_PS), Skybox_VS, sizeof(Skybox_VS));
			RShaderManager::Instance().AddShader("Default", Default_PS, sizeof(Default_PS), Default_VS, sizeof(Default_VS));

			m_DefaultShader = RShaderManager::Instance().GetShaderResource("Default");

			// Create input layout
			D3D11_INPUT_ELEMENT_DESC objVertDesc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			RRenderer.D3DDevice()->CreateInputLayout(objVertDesc, 5, m_DefaultShader->VS_Bytecode, m_DefaultShader->VS_BytecodeSize, &m_MeshInputLayout);


			m_Skybox.CreateSkybox(L"../Assets/powderpeak.dds");

			RResourceManager::Instance().LoadFbxMesh("../Assets/Sphere.fbx", m_MeshInputLayout);
			RResourceManager::Instance().LoadFbxMesh("../Assets/SpeedballPlayer.fbx", m_MeshInputLayout);
			RResourceManager::Instance().LoadFbxMesh("../Assets/AO_Scene.fbx", m_MeshInputLayout);
			RResourceManager::Instance().LoadFbxMesh("../Assets/tachikoma.fbx", m_MeshInputLayout);
			RResourceManager::Instance().LoadFbxMesh("../Assets/Island.fbx", m_MeshInputLayout);
			RResourceManager::Instance().LoadFbxMesh("../Assets/city.fbx", m_MeshInputLayout);

			// Create texture sampler state
			D3D11_SAMPLER_DESC samplerDesc;
			ZeroMemory(&samplerDesc, sizeof(samplerDesc));
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			RRenderer.D3DDevice()->CreateSamplerState(&samplerDesc, &m_SamplerState);

			m_CamYaw = m_CamPitch = 0.0f;
			m_CameraMatrix = RMatrix4::IDENTITY;

			return true;
		}

		void UpdateScene(const RTimer& timer)
		{
			if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Pressed)
			{
				RInput.HideCursor();
				RInput.LockCursor();
			}

			if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Released)
			{
				RInput.ShowCursor();
				RInput.UnlockCursor();
			}

			if (RInput.IsKeyDown(VK_RBUTTON))
			{
				int dx, dy;
				RInput.GetCursorRelPos(dx, dy);
				if (dx || dy)
				{
					m_CamYaw += (float)dx / 200.0f;
					m_CamPitch += (float)dy / 200.0f;
					m_CamPitch = max(-PI / 2, min(PI / 2, m_CamPitch));
				}
			}

			float camSpeed = 100.0f;
			if (RInput.IsKeyDown(VK_LSHIFT))
				camSpeed *= 10.0f;
			RVec3 moveVec(0.0f, 0.0f, 0.0f);
			if (RInput.IsKeyDown('W'))
				moveVec += RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
			if (RInput.IsKeyDown('S'))
				moveVec -= RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
			if (RInput.IsKeyDown('A'))
				moveVec -= RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
			if (RInput.IsKeyDown('D'))
				moveVec += RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;

			RVec3 camPos = m_CameraMatrix.GetTranslation();
			m_CameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);
			m_CameraMatrix.SetTranslation(camPos + (RVec4(moveVec, 1.0f) * m_CameraMatrix).ToVec3());

			RMatrix4 viewMatrix = m_CameraMatrix.GetViewMatrix();
			RMatrix4 projMatrix = RMatrix4::CreatePerspectiveProjectionLH(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);

			// Update scene constant buffer
			SHADER_SCENE_BUFFER cbScene;

			cbScene.viewMatrix = viewMatrix;
			cbScene.projMatrix = projMatrix;
			cbScene.viewProjMatrix = viewMatrix * projMatrix;
			cbScene.cameraPos = m_CameraMatrix.GetRow(3);

			D3D11_MAPPED_SUBRESOURCE subres;
			RRenderer.D3DImmediateContext()->Map(m_cbScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
			memcpy(subres.pData, &cbScene, sizeof(SHADER_SCENE_BUFFER));
			RRenderer.D3DImmediateContext()->Unmap(m_cbScene, 0);

			// Update object constant buffer
			SHADER_OBJECT_BUFFER cbObject;
			cbObject.worldMatrix = RMatrix4::IDENTITY;

			RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
			memcpy(subres.pData, &cbObject, sizeof(cbObject));
			RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

			RRenderer.D3DImmediateContext()->VSSetConstantBuffers(0, 1, &m_cbPerObject);
			RRenderer.D3DImmediateContext()->VSSetConstantBuffers(1, 1, &m_cbScene);
			RRenderer.D3DImmediateContext()->PSSetSamplers(0, 1, &m_SamplerState);
		}

		void RenderScene()
		{
			RRenderer.Clear();
			m_Skybox.Draw();

			RRenderer.Clear(false);
			m_MeshObject.Draw();

			RRenderer.Present();
		}

		void PreviewMesh(const char* path)
		{
			RMesh* mesh = RResourceManager::Instance().FindMesh(path);
			m_MeshObject.SetMesh(mesh);
			m_MeshObject.SetOverridingShader(m_DefaultShader);
		}
	};

	RhinoEngineWrapper::RhinoEngineWrapper()
	{
		m_IsInitialized = false;
		m_Engine = new REngine();
		m_Application = new EditorApp();
		m_Engine->BindApp(m_Application);
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

		m_IsInitialized = m_Engine->Initialize((HWND)hWnd.ToPointer(), width, height);
		return m_IsInitialized;
	}

	void RhinoEngineWrapper::RunOneFrame()
	{
		if (m_IsInitialized)
			m_Engine->RunOneFrame(true);
	}

	void RhinoEngineWrapper::Shutdown()
	{
		if (m_IsInitialized)
		{
			m_Engine->Shutdown();

			delete m_Application;
			delete m_Engine;
		}
	}

	void RhinoEngineWrapper::Resize(int width, int height)
	{
		if (m_IsInitialized)
			m_Engine->ResizeClientWindow(width, height);
	}

	List<String^>^ RhinoEngineWrapper::GetMeshNameList()
	{
		List<String^>^ list = gcnew List<String^>();

		const vector<RMesh*>& meshList = RResourceManager::Instance().GetMeshResources();
		for (vector<RMesh*>::const_iterator iter = meshList.begin(); iter != meshList.end(); iter++)
		{
			list->Add(gcnew String((*iter)->GetPath().data()));
		}

		return list;
	}

	void RhinoEngineWrapper::UpdatePreviewMesh(String^ path)
	{
		IntPtr pNativeStr = Marshal::StringToHGlobalAnsi(path);
		m_Application->PreviewMesh(static_cast<const char*>(pNativeStr.ToPointer()));
	}

	void RhinoEngineWrapper::OnKeyDown(int keycode)
	{
		RInput._SetKeyDown(keycode, true);
	}

	void RhinoEngineWrapper::OnKeyUp(int keycode)
	{
		RInput._SetKeyDown(keycode, false);
	}
}
