//=============================================================================
// FightingGameApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _FIGHTINGGAMEAPP_H
#define _FIGHTINGGAMEAPP_H

#include "Rhino.h"
#include "FTGPlayerController.h"

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
	RScene				m_Scene;
	RCamera*			m_Camera;
	RShadowMap			m_ShadowMap;

	FTGPlayerController*	m_Player;
	FTGPlayerController*	m_AIPlayer;

	bool				m_DrawHitBound;
	RText				m_Text;
};

#endif
