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

#include "EditorAxis.h"

#pragma comment(lib, "User32.lib")

#include <direct.h>

using namespace System::Runtime::InteropServices;

extern REngine* gEngine;

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
		
		vector<RSMeshObject*>		m_MeshObjects;
		RSMeshObject*				m_SelectedObject;

		ID3D11InputLayout*			m_MeshInputLayout;
		RShader*					m_DefaultShader;
		RMatrix4					m_CameraMatrix;
		RMatrix4					m_InvViewProjMatrix;
		float						m_CameraX, m_CameraY;
		RVec3						m_MeshPos;
		float						m_CamFov;

		struct PRIMITIVE_VERTEX
		{
			RVec4	pos;
			RColor	color;
		};

		RShader*					m_ColorShader;
		vector<PRIMITIVE_VERTEX>	m_PrimitiveList;
		RMeshElement				m_PrimitiveMeshBuffer;
		ID3D11InputLayout*			m_PrimitiveInputLayout;

		EditorAxis					m_EditorAxis;
		int							m_MouseDownX, m_MouseDownY;
		RMatrix4					m_AxisMatrix;

		enum MouseControlMode
		{
			MCM_NONE,
			MCM_MOVE_X,
			MCM_MOVE_Y,
			MCM_MOVE_Z,
		};

		MouseControlMode			m_MouseControlMode;

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

			m_EditorAxis.Release();

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

			//m_XAxisMeshBuffer.

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
			m_SelectedObject = nullptr;
			m_MouseControlMode = MCM_NONE;

			m_EditorAxis.Create();

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

			if (RInput.GetBufferedKeyState(VK_LBUTTON) == BKS_Pressed)
			{
				RECT rect = gEngine->GetWindowRectInfo();
				int cur_x, cur_y;

				RInput.GetCursorPos(cur_x, cur_y);
				float fx = float(cur_x - rect.left) / float(rect.right - rect.left),
					  fy = float(cur_y - rect.top) / float(rect.bottom - rect.top);

				RunScreenToCameraRayPicking(fx, fy);
			}

			if (RInput.GetBufferedKeyState(VK_LBUTTON) == BKS_Released)
			{
				m_MouseControlMode = MCM_NONE;
			}

			if (m_MouseControlMode != MCM_NONE)
			{
				if (m_SelectedObject)
				{
					int mdx, mdy;
					RInput.GetCursorRelPos(mdx, mdy);

					RVec3 obj_forward = m_SelectedObject->GetNodeTransform().GetForward();
					RVec3 obj_up = m_SelectedObject->GetNodeTransform().GetUp();
					RVec3 obj_right = m_SelectedObject->GetNodeTransform().GetRight();

					RVec3 cam_right = m_CameraMatrix.GetRight();
					RVec3 cam_up = m_CameraMatrix.GetUp();

					RVec3 pos = m_SelectedObject->GetPosition();
					if (m_MouseControlMode == MCM_MOVE_X)
					{
						pos.x += obj_right.Dot(cam_right) * mdx - obj_right.Dot(cam_up) * mdy;
					}
					else if (m_MouseControlMode == MCM_MOVE_Y)
					{
						pos.y += obj_up.Dot(cam_right) * mdx - obj_up.Dot(cam_up) * mdy;
					}
					else if (m_MouseControlMode == MCM_MOVE_Z)
					{
						pos.z += obj_forward.Dot(cam_right) * mdx - obj_forward.Dot(cam_up) * mdy;
					}
					m_SelectedObject->SetPosition(pos);
				}
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

			if (m_SelectedObject)
			{
				const RAabb& aabb = m_SelectedObject->GetAabb();
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
						RColor(0.0f, 1.0f, 0.0f),
					};
					m_PrimitiveList.push_back(v);
				}
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
			for (vector<RSMeshObject*>::iterator iter = m_MeshObjects.begin(); iter != m_MeshObjects.end(); iter++)
			{
				cbObject.worldMatrix = (*iter)->GetNodeTransform();

				RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
				memcpy(subres.pData, &cbObject, sizeof(cbObject));
				RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

				(*iter)->Draw();
			}

			cbObject.worldMatrix = RMatrix4::IDENTITY;

			RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
			memcpy(subres.pData, &cbObject, sizeof(cbObject));
			RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

			if (m_PrimitiveList.size())
			{
				if (m_SelectedObject)
					cbObject.worldMatrix = m_SelectedObject->GetNodeTransform();
				else
					cbObject.worldMatrix = RMatrix4::IDENTITY;

				RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
				memcpy(subres.pData, &cbObject, sizeof(cbObject));
				RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

				m_ColorShader->Bind();
				RRenderer.D3DImmediateContext()->IASetInputLayout(m_PrimitiveInputLayout);
				m_PrimitiveMeshBuffer.Draw(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			}

			RRenderer.Clear(false);

			// Draw axises
			if (m_SelectedObject)
			{
				m_ColorShader->Bind();

				RVec3 cam_pos = m_CameraMatrix.GetTranslation();
				RVec3 obj_pos = m_SelectedObject->GetPosition();
				float dist = (cam_pos - obj_pos).Magnitude();
				dist = max(50.0f, min(100.0f, dist));
				RVec3 axis_pos = cam_pos + (m_SelectedObject->GetPosition() - cam_pos).GetNormalizedVec3() * dist;
				
				m_AxisMatrix = RMatrix4::CreateTranslation(axis_pos);
				cbObject.worldMatrix = m_AxisMatrix;

				RRenderer.D3DImmediateContext()->Map(m_cbPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &subres);
				memcpy(subres.pData, &cbObject, sizeof(cbObject));
				RRenderer.D3DImmediateContext()->Unmap(m_cbPerObject, 0);

				RRenderer.D3DImmediateContext()->IASetInputLayout(m_PrimitiveInputLayout);
				m_EditorAxis.Draw();
			}

			RRenderer.Present();
		}

		void AddMeshObjectToScene(const char* path)
		{
			RSMeshObject* meshObj = new RSMeshObject();
			RMesh* mesh = RResourceManager::Instance().FindMesh(path);
			meshObj->SetMesh(mesh);
			meshObj->SetOverridingShader(m_DefaultShader);
			m_MeshObjects.push_back(meshObj);
		}

		void RunScreenToCameraRayPicking(float x, float y)
		{
			RVec3 farPoint = RVec3(2.0f * x - 1.0f, -2.0f * y + 1.0f, 1.0f);
			RVec4 farPointVec4 = RVec4(farPoint) * m_InvViewProjMatrix;
			RVec3 farPointWorld = (farPointVec4 / farPointVec4.w).ToVec3();
			RVec3 camPos = m_CameraMatrix.GetTranslation();

			RRay ray(camPos, farPointWorld);
			RRay axis_ray = ray;

			if (m_SelectedObject)
			{
				axis_ray = ray.Transform(m_AxisMatrix.GetViewMatrix());
			}

			m_MouseControlMode = MCM_NONE;

			if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis.GetAabb(AXIS_X)))
			{
				m_MouseControlMode = MCM_MOVE_X;
			}
			else if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis.GetAabb(AXIS_Y)))
			{
				m_MouseControlMode = MCM_MOVE_Y;
			}
			else if (m_SelectedObject && axis_ray.TestAabbIntersection(m_EditorAxis.GetAabb(AXIS_Z)))
			{
				m_MouseControlMode = MCM_MOVE_Z;
			}
			else
			{
				m_SelectedObject = nullptr;

				for (vector<RSMeshObject*>::iterator iter = m_MeshObjects.begin(); iter != m_MeshObjects.end(); iter++)
				{
					RRay local_ray = ray.Transform((*iter)->GetNodeTransform().GetViewMatrix());

					if (local_ray.TestAabbIntersection((*iter)->GetAabb()))
					{
						m_SelectedObject = *iter;
					}
				}
			}

			if (m_MouseControlMode != MCM_NONE)
			{
				RInput.GetCursorPos(m_MouseDownX, m_MouseDownY);
			}
		}

		void DeleteSelection()
		{
			if (m_SelectedObject)
			{
				vector<RSMeshObject*>::iterator iter = std::find(m_MeshObjects.begin(), m_MeshObjects.end(), m_SelectedObject);
				m_MeshObjects.erase(iter);
				delete m_SelectedObject;
				m_SelectedObject = nullptr;
			}
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
		m_Application->AddMeshObjectToScene(static_cast<const char*>(pNativeStr.ToPointer()));
	}

	void RhinoEngineWrapper::OnKeyDown(int keycode)
	{
		RInput._SetKeyDown(keycode, true);
	}

	void RhinoEngineWrapper::OnKeyUp(int keycode)
	{
		RInput._SetKeyDown(keycode, false);
	}

	void RhinoEngineWrapper::RunScreenToCameraRayPicking(float x, float y)
	{
		//m_Application->RunScreenToCameraRayPicking(x, y);
	}

	void RhinoEngineWrapper::DeleteSelection()
	{
		m_Application->DeleteSelection();
	}
}
