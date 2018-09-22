//=============================================================================
// EditorApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "EditorAxis.h"


namespace ManagedEngineWrapper
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

		EditorAxis					m_EditorAxis;
		int							m_MouseDownX, m_MouseDownY;
		RMatrix4					m_AxisMatrix;

		enum class MouseControlMode
		{
			None,
			MoveX,
			MoveY,
			MoveZ,
		};

		MouseControlMode			m_MouseControlMode;

	public:
		EditorApp();
		~EditorApp();

		virtual bool Initialize() override;

		virtual void UpdateScene(const RTimer& timer) override;
		virtual void RenderScene() override;

		void AddMeshObjectToScene(const char* path);
		void LoadScene(const char* filename);
		void SaveScene(const char* filename);

		RSceneObject* GetSelection();

		void SaveMeshMaterialFromSelection();
		void ExportAllAnimationsToBinaryFiles();

		void RunScreenToCameraRayPicking(float x, float y);

		bool DeleteSelection();

	private:
		float SnapTo(float Value, float Unit);
	};
#pragma managed
}