//=============================================================================
// SimpleGame.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Rhino.h"

class RgRubik;

class SimpleGame : public IApp
{
public:
	SimpleGame();
	virtual ~SimpleGame() override;

	virtual bool Initialize() override;
	virtual void UpdateScene(const RTimer& timer) override;

	virtual void OnResize(int width, int height) override;
	virtual TCHAR* WindowTitle() override;

private:
	void SetupScene();

	// Begin scrambling the cube. The cube faces will randomly rotate couple times.
	void ScrambleCube();

private:
	float m_CamYaw;
	float m_CamPitch;
	RCamera* m_Camera;

	RSkybox m_Skybox;

	RgRubik*		m_RubikCube;
	RSceneObject*	m_CameraOrbiter;
	bool			m_IsScramblingCube;
	std::vector<UINT8>	m_ScrambleMoves;
};

