//=============================================================================
// FightingGameApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _FIGHTINGGAMEAPP_H
#define _FIGHTINGGAMEAPP_H

#include "Rhino.h"
#include "PlayerController.h"
#include "RText.h"

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
	RCamera				m_Camera;
	RShadowMap			m_ShadowMap;

	PlayerController*	m_Player;
	PlayerController*	m_AIPlayer;

	RDebugRenderer		m_DebugRenderer;
	bool				m_DrawHitBound;
	RText				m_Text;
};

#endif
