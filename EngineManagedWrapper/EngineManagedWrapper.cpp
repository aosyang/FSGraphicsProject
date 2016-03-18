// This is the main DLL file.

#include "stdafx.h"
#include "Rhino.h"

#include "EngineManagedWrapper.h"
#include "RSkybox.h"

#include "Skybox_PS.csh"
#include "Skybox_VS.csh"

#include "ConstBufferPS.h"
#include "ConstBufferVS.h"

#pragma comment(lib, "User32.lib")

#include <direct.h>

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
	public:
		~EditorApp()
		{
			m_Skybox.Release();
			SAFE_RELEASE(m_cbPerObject);
			SAFE_RELEASE(m_cbScene);
			SAFE_RELEASE(m_SamplerState);

			RShaderManager::Instance().UnloadAllShaders();
			RResourceManager::Instance().Destroy();
		}

		bool Initialize()
		{
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

			m_Skybox.CreateSkybox(L"../Assets/powderpeak.dds");


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

			RMatrix4 cameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);
			cameraMatrix.SetTranslation(RVec3(0.0f, 0.0f, 0.0f));

			RMatrix4 viewMatrix = cameraMatrix.GetViewMatrix();
			RMatrix4 projMatrix = RMatrix4::CreatePerspectiveProjectionLH(65.0f, RRenderer.AspectRatio(), 1.0f, 10000.0f);

			// Update scene constant buffer
			SHADER_SCENE_BUFFER cbScene;

			cbScene.viewMatrix = viewMatrix;
			cbScene.projMatrix = projMatrix;
			cbScene.viewProjMatrix = viewMatrix * projMatrix;
			cbScene.cameraPos = cameraMatrix.GetRow(3);

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
			RRenderer.Present();
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
}
