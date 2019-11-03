//=============================================================================
// RDebugMenu.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

class RDebugMenu : public RText
{
public:
	RDebugMenu();
	~RDebugMenu();

	void AddFloatMenuItem(const char* name, float* val, float step = 0.1f);
	void AddIntMenuItem(const char* name, int* val, int step = 1);
	void AddBoolMenuItem(const char* name, bool* val);
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

		enum ItemValueType
		{
			Item_Float,
			Item_Int,
			Item_Bool,
		};

		ItemValueType type;

		union
		{
			float*	fval;
			int*	ival;
			bool*	bval;
		};

		union
		{
			float	fstep;
			int		istep;
		};

		void Increase();
		void Decrease();
	};

	std::vector<MenuItem>		m_MenuItems;
	UINT					m_SelMenu;
	bool					m_bEnabled;
	float					m_KeyRepeatTime;
};

