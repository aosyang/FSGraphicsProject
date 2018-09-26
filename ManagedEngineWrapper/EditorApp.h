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
		float						m_CamYaw, m_CamPitch;

		RScene						m_Scene;
		RCamera*					m_EditorCamera;

		RSceneObject*				m_SelectedObject;

		RShader*					m_DefaultShader;

		// For mouse picking from viewport
		RMatrix4					m_InvViewProjMatrix;

		EditorAxis*					m_EditorAxis;
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

		void CreateEditorObjects();

		virtual void UpdateScene(const RTimer& timer) override;
		virtual void RenderScene() override {}

		virtual void OnResize(int width, int height) override;

		RSceneObject* AddMeshObjectToScene(const char* MeshAssetPath);
		void LoadScene(const char* filename);
		void SaveScene(const char* filename);

		RSceneObject* GetSelection();
		void SetSelection(RSceneObject* SceneObject);
		vector<RSceneObject*> GetSceneObjects() const;

		void SaveMeshMaterialFromSelection();
		void ExportAllAnimationsToBinaryFiles();

		void RunScreenToCameraRayPicking(float x, float y);

		bool DeleteSelection();

	private:
		float SnapTo(float Value, float Unit);
	};
#pragma managed
}