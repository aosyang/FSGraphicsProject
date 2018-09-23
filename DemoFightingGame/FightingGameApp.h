//=============================================================================
// FightingGameApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"
#include "FTGPlayerController.h"
#include "AIFighterLogic.h"

const static int MaxNumAIs = 5;

class FightingGameApp : public IApp
{
public:
	FightingGameApp();
	~FightingGameApp();

	virtual bool Initialize() override;
	virtual void UpdateScene(const RTimer& timer) override;
	virtual void RenderScene() override;

	virtual void OnResize(int width, int height) override;
	virtual TCHAR* WindowTitle() override { return L"Fighting Game Demo"; }

private:

	void UpdateCameraPosition(float DeltaTime);

	void ResetPlayerPosition(FTGPlayerController* PlayerController);

private:
	RScene				m_Scene;
	RCamera*			m_Camera;
	RShadowMap			m_ShadowMap;

	FTGPlayerController*	m_Player;
	FTGPlayerController*	m_AIPlayer[MaxNumAIs];
	AIFighterLogic*			m_AILogic[MaxNumAIs];

	RText				m_Text;
};

