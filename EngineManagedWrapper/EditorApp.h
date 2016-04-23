//=============================================================================
// EditorApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"

#include "EditorAxis.h"


extern REngine* gEngine;

namespace EngineManagedWrapper
{
#pragma unmanaged
	class EditorApp : public IApp
	{
	private:
		RSkybox						m_Skybox;
		ID3D11SamplerState*			m_SamplerState;
		ID3D11SamplerState*			m_SamplerComparisonState;
		float						m_CamYaw, m_CamPitch;

		RScene						m_Scene;

		RSceneObject*				m_SelectedObject;

		RShader*					m_DefaultShader;
		RMatrix4					m_CameraMatrix;
		RMatrix4					m_InvViewProjMatrix;
		float						m_CameraX, m_CameraY;
		RVec3						m_MeshPos;
		float						m_CamFov;

		RShader*					m_ColorShader;
		RDebugRenderer				m_DebugRenderer;

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
		EditorApp();
		~EditorApp();

		bool Initialize();

		void UpdateScene(const RTimer& timer);
		void RenderScene();

		void AddMeshObjectToScene(const char* path);
		void LoadScene(const char* filename);
		void SaveScene(const char* filename);
		void SaveMeshMaterialFromSelection();

		void RunScreenToCameraRayPicking(float x, float y);

		void DeleteSelection();
	};
#pragma managed
}