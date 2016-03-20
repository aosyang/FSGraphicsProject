// This is the main DLL file.

#include "stdafx.h"
#include "Rhino.h"

#include "EngineManagedWrapper.h"
#include "RSkybox.h"

#include "Skybox_PS.csh"
#include "Skybox_VS.csh"
#include "Default_PS.csh"
#include "Default_VS.csh"
#include "Color_PS.csh"
#include "Color_VS.csh"

#include "ConstBufferPS.h"
#include "ConstBufferVS.h"

#pragma comment(lib, "User32.lib")

#include <direct.h>

using namespace System::Runtime::InteropServices;

namespace EngineManagedWrapper
{
#pragma unmanaged
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
		RMatrix4					m_InvViewProjMatrix;
		float						m_CameraX, m_CameraY;
		RVec3						m_MeshPos;
		float						m_CamFov;

		struct PRIMITIVE_VERTEX
		{
			RVec4 pos;
			RVec4 color;
		};

		RShader*					m_ColorShader;
		vector<PRIMITIVE_VERTEX>	m_PrimitiveList;
		RMeshElement				m_PrimitiveMeshBuffer;
		ID3D11InputLayout*			m_PrimitiveInputLayout;

		bool						m_DrawBoundingBox;

	public:
		~EditorApp()
		{
			m_Skybox.Release();
			SAFE_RELEASE(m_cbPerObject);
			SAFE_RELEASE(m_cbScene);
			SAFE_RELEASE(m_SamplerState);
			SAFE_RELEASE(m_MeshInputLayout);
			SAFE_RELEASE(m_PrimitiveInputLayout);
			m_PrimitiveMeshBuffer.Release();

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
			RShaderManager::Instance().AddShader("Color", Color_PS, sizeof(Color_PS), Color_VS, sizeof(Color_VS));

			m_DefaultShader = RShaderManager::Instance().GetShaderResource("Default");
			m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");

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


			D3D11_INPUT_ELEMENT_DESC primitiveVertDesc[] =
			{
				{ "POSITION",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			RRenderer.D3DDevice()->CreateInputLayout(primitiveVertDesc, 2, m_ColorShader->VS_Bytecode, m_ColorShader->VS_BytecodeSize, &m_PrimitiveInputLayout);
			m_PrimitiveMeshBuffer.CreateVertexBuffer(nullptr, sizeof(PRIMITIVE_VERTEX), 65536, true);

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

			m_CamFov = 65.0f;
			m_CamYaw = m_CamPitch = 0.0f;
			m_CameraMatrix = RMatrix4::IDENTITY;
			m_MeshPos = RVec3::Zero();
			m_DrawBoundingBox = false;

			return true;
		}

		void UpdateScene(const RTimer& timer)
		{
			m_PrimitiveList.clear();

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

			RVec3 moveVec(0.0f, 0.0f, 0.0f);

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


				float camSpeed = 100.0f;
				if (RInput.IsKeyDown(VK_LSHIFT))
					camSpeed *= 10.0f;

				if (RInput.IsKeyDown('W'))
					moveVec += RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
				if (RInput.IsKeyDown('S'))
					moveVec -= RVec3(0.0f, 0.0f, 1.0f) * timer.DeltaTime() * camSpeed;
				if (RInput.IsKeyDown('A'))
					moveVec -= RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
				if (RInput.IsKeyDown('D'))
					moveVec += RVec3(1.0f, 0.0f, 0.0f) * timer.DeltaTime() * camSpeed;
			}

			RVec3 camPos = m_CameraMatrix.GetTranslation();
			m_CameraMatrix = RMatrix4::CreateXAxisRotation(m_CamPitch * 180 / PI) * RMatrix4::CreateYAxisRotation(m_CamYaw * 180 / PI);
			m_CameraMatrix.SetTranslation(camPos + (RVec4(moveVec, 1.0f) * m_CameraMatrix).ToVec3());

			RMatrix4 viewMatrix = m_CameraMatrix.GetViewMatrix();
			RMatrix4 projMatrix = RMatrix4::CreatePerspectiveProjectionLH(m_CamFov, RRenderer.AspectRatio(), 1.0f, 10000.0f);
			RMatrix4 viewProjMatrix = viewMatrix * projMatrix;

			m_InvViewProjMatrix = viewProjMatrix.Inverse();

			const RAabb& aabb = m_MeshObject.GetAabb();
			RVec3 cornerPoints[] = 
			{
				RVec3(aabb.pMin.x, aabb.pMin.y, aabb.pMin.z),
				RVec3(aabb.pMin.x, aabb.pMin.y, aabb.pMax.z),
				RVec3(aabb.pMin.x, aabb.pMax.y, aabb.pMax.z),
				RVec3(aabb.pMin.x, aabb.pMax.y, aabb.pMin.z),

				RVec3(aabb.pMax.x, aabb.pMin.y, aabb.pMin.z),
				RVec3(aabb.pMax.x, aabb.pMin.y, aabb.pMax.z),
				RVec3(aabb.pMax.x, aabb.pMax.y, aabb.pMax.z),
				RVec3(aabb.pMax.x, aabb.pMax.y, aabb.pMin.z),
			};

			int wiredCubeIdx[] =
			{
				0, 1, 1, 2, 2, 3, 3, 0,
				4, 5, 5, 6, 6, 7, 7, 4,
				0, 4, 1, 5, 2, 6, 3, 7,
			};

			for (int i = 0; i < 24; i++)
			{
				PRIMITIVE_VERTEX v =
				{
					RVec4(cornerPoints[wiredCubeIdx[i]]),
					RVec4(0.0f, 1.0f, 0.0f),
				};
				m_PrimitiveList.push_back(v);
			}

			// Update scene constant buffer
			SHADER_SCENE_BUFFER cbScene;

			cbScene.viewMatrix = viewMatrix;
			cbScene.projMatrix = projMatrix;
			cbScene.viewProjMatrix = viewProjMatrix;
			cbScene.cameraPos = m_CameraMatrix.GetRow(3);

			D3D11_MAPPED_SUBRESOURCE subres;
			RRenderer.D3DImmediateContext()->Map(m_cbScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
			memcpy(subres.pData, &cbScene, sizeof(SHADER_SCENE_BUFFER));
			RRenderer.D3DImmediateContext()->Unmap(m_cbScene, 0);

			m_PrimitiveMeshBuffer.UpdateDynamicVertexBuffer(m_PrimitiveList.data(), sizeof(PRIMITIVE_VERTEX), m_PrimitiveList.size());

			RRenderer.D3DImmediateContext()->VSSetConstantBuffers(0, 1, &m_cbPerObject);
			RRenderer.D3DImmediateContext()->VSSetConstantBuffers(1, 1, &m_cbScene);
			RRenderer.D3DImmediateContext()->PSSetSamplers(0, 1, &m_SamplerState);
		}

		void RenderScene()
		{
			RRenderer.Clear();

			// Update object constant buffer
			SHADER_OBJECT_BUFFER cbObject;
			cbObject.worldMatrix = RMatrix4::IDENTITY;

			D3D11_MAPPED_SUBRESOURCE subres;
			RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
			memcpy(subres.pData, &cbObject, sizeof(cbObject));
			RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

			m_Skybox.Draw();

			RRenderer.Clear(false);

			// Update object constant buffer
			cbObject.worldMatrix = RMatrix4::CreateTranslation(m_MeshPos);

			RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
			memcpy(subres.pData, &cbObject, sizeof(cbObject));
			RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

			m_MeshObject.Draw();

			if (m_PrimitiveList.size() && m_DrawBoundingBox)
			{
				cbObject.worldMatrix = RMatrix4::IDENTITY;

				RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
				memcpy(subres.pData, &cbObject, sizeof(cbObject));
				RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

				m_ColorShader->Bind();
				RRenderer.D3DImmediateContext()->IASetInputLayout(m_PrimitiveInputLayout);
				m_PrimitiveMeshBuffer.Draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			}

			RRenderer.Present();
		}

		void PreviewMesh(const char* path)
		{
			RMesh* mesh = RResourceManager::Instance().FindMesh(path);
			m_MeshObject.SetMesh(mesh);
			m_MeshObject.SetOverridingShader(m_DefaultShader);
		}

		void ScreenToCameraRay(float x, float y)
		{
			RVec3 farPoint = RVec3(2.0f * x - 1.0f, -2.0f * y + 1.0f, 1.0f);
			RVec4 farPointVec4 = RVec4(farPoint) * m_InvViewProjMatrix;
			RVec3 farPointWorld = (farPointVec4 / farPointVec4.w).ToVec3();
			RVec3 camPos = m_CameraMatrix.GetTranslation();

			RRay ray(camPos, farPointWorld);
			m_DrawBoundingBox = ray.TestAabbIntersection(m_MeshObject.GetAabb());
		}
	};
#pragma managed

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

	void RhinoEngineWrapper::ScreenToCameraRay(float x, float y)
	{
		m_Application->ScreenToCameraRay(x, y);
	}
}
