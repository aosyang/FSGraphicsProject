//=============================================================================
// FightingGameApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"
#include "RhinoGameUtils.h"

#include "FTGPlayerController.h"

const static int MaxNumAIs = 10;

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

	void UpdateUserInput();

	void UpdateCameraPosition(float DeltaTime);

	void ResetPlayerPosition(FTGPlayerController* PlayerController);

	FTGPlayerController* GetCurrentPlayer() const;

private:
	RCamera*				m_Camera;
	RShadowMap				m_ShadowMap;

	FTGPlayerController*	m_Player[2];
	int						CurrentPlayerIndex;

	std::vector<FTGPlayerController*>	m_AIPlayers;

	RText					m_Text;
	bool					m_FreeFlyMode;
	RFreeFlyCameraControl*	m_FreeFlyCameraControl;
};

