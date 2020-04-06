//=============================================================================
// FightingGameApp.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Rhino.h"
#include "RhinoGameUtils.h"

#include "FTGPlayerController.h"

class FightingGameApp : public IApp
{
public:
	FightingGameApp();
	~FightingGameApp();

	virtual bool Initialize() override;
	virtual void UpdateScene(const RTimer& timer) override;

	virtual void OnResize(int width, int height) override;
	virtual TCHAR* WindowTitle() override { return L"Fighting Game Demo"; }

private:

	void UpdateUserInput();

	void UpdateCameraPosition(float DeltaTime);

	void CreatePhysicsBoxes();

	void ResetPlayerPosition(FTGPlayerController* PlayerController);

	PlayerControllerBase* GetCurrentPlayer() const;

private:
	RCamera*				m_Camera;

	std::vector<PlayerControllerBase*>	m_Players;
	int						CurrentPlayerIndex;

	std::vector<FTGPlayerController*>	m_AIPlayers;

	RText					m_Text;
	bool					m_FreeFlyMode;
	RFreeFlyCameraControl*	m_FreeFlyCameraControl;
};

