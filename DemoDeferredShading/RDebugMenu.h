//=============================================================================
// RDebugMenu.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RDEBUGMENU_H
#define _RDEBUGMENU_H

#include "Rhino.h"

struct MenuItem
{
	char	name[256];
	float*	val;
	float	step;
};

class RDebugMenu : public RText
{
public:
	RDebugMenu();
	~RDebugMenu();

	void AddMenuItem(const char* name, float* val, float step = 0.1f);
	void Update();
	void Render();

	void SetEnabled(bool enabled) { m_bEnabled = enabled; }
	bool GetEnabled() const { return m_bEnabled; }
private:
	vector<MenuItem>		m_MenuItems;
	UINT					m_SelMenu;
	bool					m_bEnabled;
};

#endif
