//=============================================================================
// SimpleGame.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "Rhino.h"

class SimpleGame : public IApp
{
public:
	SimpleGame();
	virtual ~SimpleGame() override;

	virtual bool Initialize() override;
	virtual void UpdateScene(const RTimer& timer) override;
	virtual void RenderScene() override;

private:
	void SetupScene();

private:
	RScene	m_Scene;

	float m_CamYaw;
	float m_CamPitch;
	RCamera m_Camera;

	RSkybox m_Skybox;
};

