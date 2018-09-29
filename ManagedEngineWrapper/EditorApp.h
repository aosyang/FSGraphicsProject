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
		RMatrix4					m_AxisMatrix;

		/// The cursor position in world space when object moving starts
		RVec3						m_CursorStartPosition;

		/// The object position in world space when object moving starts
		RVec3						m_ObjectStartPosition;

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

		void RunScreenToCameraRayPicking(const RVec2& Point);

		bool DeleteSelection();

	private:
		float SnapTo(float Value, float Unit);

		/// Make a ray from a point in the viewport. X and Y of the point are ranged in [0..1]
		RRay MakeRayFromViewportPoint(const RVec2& Point);

		/// Get the relative cursor point in the viewport
		RVec2 GetCursorPointInViewport() const;

		/// Get the plane facing camera from a given point and axis
		RPlane GetAxisPlane(const RVec3& Point, const RVec3& AxisDirection) const;
	};
#pragma managed
}