//=============================================================================
// RDebugMenu.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#ifndef _RDEBUGMENU_H
#define _RDEBUGMENU_H

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
	bool IsKeyDownOrRepeat(int keycode);

private:
	struct MenuItem
	{
		char	name[256];
		float*	val;
		float	step;
	};

	vector<MenuItem>		m_MenuItems;
	UINT					m_SelMenu;
	bool					m_bEnabled;
	float					m_KeyRepeatTime;
};

#endif
