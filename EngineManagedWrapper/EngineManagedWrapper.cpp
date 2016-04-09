// This is the main DLL file.

#include "stdafx.h"
#include "Rhino.h"

#include "EngineManagedWrapper.h"

#include "../Shaders/ConstBufferPS.h"
#include "../Shaders/ConstBufferVS.h"

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
		RShaderConstantBuffer<SHADER_OBJECT_BUFFER,		CBST_VS, 0>				m_cbPerObject;
		RShaderConstantBuffer<SHADER_SCENE_BUFFER,		CBST_VS|CBST_GS, 1>		m_cbScene;
		RShaderConstantBuffer<SHADER_LIGHT_BUFFER,		CBST_PS, 0>				m_cbLight;
		ID3D11SamplerState*			m_SamplerState;
		ID3D11SamplerState*			m_SamplerComparisonState;
		float						m_CamYaw, m_CamPitch;
		
		vector<RSMeshObject*>		m_MeshObjects;
		RSMeshObject*				m_SelectedObject;

		RShader*					m_DefaultShader;
		RMatrix4					m_CameraMatrix;
		RMatrix4					m_InvViewProjMatrix;
		float						m_CameraX, m_CameraY;
		RVec3						m_MeshPos;
		float						m_CamFov;

		RShader*					m_ColorShader;
		vector<RVertex::PRIMITIVE_VERTEX>
									m_PrimitiveList;
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
			m_cbPerObject.Release();
			m_cbScene.Release();
			m_cbLight.Release();

			SAFE_RELEASE(m_SamplerState);
			SAFE_RELEASE(m_SamplerComparisonState);
			m_PrimitiveMeshBuffer.Release();

			m_EditorAxis.Release();

			RShaderManager::Instance().UnloadAllShaders();
			RResourceManager::Instance().Destroy();
		}

		bool Initialize()
		{
			RResourceManager::Instance().Initialize();

			m_cbPerObject.Initialize();
			m_cbScene.Initialize();
			m_cbLight.Initialize();

			RShaderManager::Instance().LoadShaders("../Shaders");

			m_DefaultShader = RShaderManager::Instance().GetShaderResource("Default");
			m_ColorShader = RShaderManager::Instance().GetShaderResource("Color");

			m_PrimitiveInputLayout = RRenderer.GetInputLayout(RVertex::PRIMITIVE_VERTEX::GetTypeName());
			m_PrimitiveMeshBuffer.CreateVertexBuffer(nullptr, sizeof(RVertex::PRIMITIVE_VERTEX), 65536, true);

#if 1
			RResourceManager::Instance().LoadAllResources();
#else
			RResourceManager::Instance().LoadFbxMesh("../Assets/Sphere.fbx");
			RResourceManager::Instance().LoadFbxMesh("../Assets/SpeedballPlayer.fbx");
			RResourceManager::Instance().LoadFbxMesh("../Assets/AO_Scene.fbx");
			RResourceManager::Instance().LoadFbxMesh("../Assets/tachikoma.fbx");
			RResourceManager::Instance().LoadFbxMesh("../Assets/Island.fbx");
			RResourceManager::Instance().LoadFbxMesh("../Assets/city.fbx");
#endif

			m_Skybox.CreateSkybox("../Assets/powderpeak.dds");

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

			ZeroMemory(&samplerDesc, sizeof(samplerDesc));
			samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			RRenderer.D3DDevice()->CreateSamplerState(&samplerDesc, &m_SamplerComparisonState);


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

			RVec3 moveVec(0.0f, 0.0f, 0.0f);

			RECT rwRect = gEngine->GetWindowRectInfo();
			int mx, my;
			RInput.GetCursorPos(mx, my);

			if (mx >= rwRect.left && mx <= rwRect.right &&
				my >= rwRect.top && my <= rwRect.bottom)
			{
				if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Pressed)
				{
					RInput.HideCursor();
					RInput.LockCursor();
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
			}

			if (RInput.GetBufferedKeyState(VK_RBUTTON) == BKS_Released)
			{
				RInput.ShowCursor();
				RInput.UnlockCursor();
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
					RVertex::PRIMITIVE_VERTEX v =
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

			m_cbScene.UpdateContent(&cbScene);

			// Update light constant buffer
			SHADER_LIGHT_BUFFER cbLight;
			ZeroMemory(&cbLight, sizeof(cbLight));

			// Setup ambient color
			cbLight.HighHemisphereAmbientColor = RVec4(1.0f, 1.0f, 1.0f, 1.0f);
			cbLight.LowHemisphereAmbientColor = RVec4(0.2f, 0.2f, 0.2f, 1.0f);

			m_cbLight.UpdateContent(&cbLight);

			m_PrimitiveMeshBuffer.UpdateDynamicVertexBuffer(m_PrimitiveList.data(), sizeof(RVertex::PRIMITIVE_VERTEX), m_PrimitiveList.size());

			m_cbPerObject.ApplyToShaders();
			m_cbScene.ApplyToShaders();
			m_cbLight.ApplyToShaders();

			RRenderer.D3DImmediateContext()->PSSetSamplers(0, 1, &m_SamplerState);
			RRenderer.D3DImmediateContext()->PSSetSamplers(2, 1, &m_SamplerComparisonState);
		}

		void RenderScene()
		{
			RRenderer.Clear();

			// Update object constant buffer
			SHADER_OBJECT_BUFFER cbObject;
			cbObject.worldMatrix = RMatrix4::IDENTITY;

			m_cbPerObject.UpdateContent(&cbObject);

			m_Skybox.Draw();

			RRenderer.Clear(false);

			// Update object constant buffer
			for (vector<RSMeshObject*>::iterator iter = m_MeshObjects.begin(); iter != m_MeshObjects.end(); iter++)
			{
				cbObject.worldMatrix = (*iter)->GetNodeTransform();

				m_cbPerObject.UpdateContent(&cbObject);

				(*iter)->Draw();
			}

			cbObject.worldMatrix = RMatrix4::IDENTITY;

			m_cbPerObject.UpdateContent(&cbObject);

			if (m_PrimitiveList.size())
			{
				if (m_SelectedObject)
					cbObject.worldMatrix = m_SelectedObject->GetNodeTransform();
				else
					cbObject.worldMatrix = RMatrix4::IDENTITY;

				m_cbPerObject.UpdateContent(&cbObject);

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

				m_cbPerObject.UpdateContent(&cbObject);

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
				vector<RSMeshObject*> rayPickingList;

				for (vector<RSMeshObject*>::iterator iter = m_MeshObjects.begin(); iter != m_MeshObjects.end(); iter++)
				{
					RRay local_ray = ray.Transform((*iter)->GetNodeTransform().GetViewMatrix());

					if (local_ray.TestAabbIntersection((*iter)->GetAabb()))
					{
						rayPickingList.push_back(*iter);
					}
				}

				if (!rayPickingList.size())
				{
					m_SelectedObject = nullptr;
				}
				else
				{
					vector<RSMeshObject*>::iterator iter = find(rayPickingList.begin(), rayPickingList.end(), m_SelectedObject);
					if (iter != rayPickingList.end())
					{
						iter++;
						if (iter == rayPickingList.end())
							iter = rayPickingList.begin();
						m_SelectedObject = *iter;
					}
					else
					{
						m_SelectedObject = rayPickingList[0];
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
