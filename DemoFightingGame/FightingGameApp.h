//=============================================================================
// FightingGameApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _FIGHTINGGAMEAPP_H
#define _FIGHTINGGAMEAPP_H

#include "Rhino.h"
#include "FTGPlayerController.h"
#include "AIFighterLogic.h"

const static int MaxNumAIs = 5;

class FightingGameApp : public IApp
{
public:
	FightingGameApp();
	~FightingGameApp();

	bool Initialize();
	void UpdateScene(const RTimer& timer);
	void RenderScene();

	void OnResize(int width, int height);
	TCHAR* WindowTitle() { return L"Fighting Game Demo"; }

private:

	void UpdateCameraPosition(float DeltaTime);

private:
	RScene				m_Scene;
	RCamera*			m_Camera;
	RShadowMap			m_ShadowMap;

	FTGPlayerController*	m_Player;
	FTGPlayerController*	m_AIPlayer[MaxNumAIs];
	AIFighterLogic*			m_AILogic[MaxNumAIs];

	RText				m_Text;
};

#endif
