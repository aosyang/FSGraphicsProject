//=============================================================================
// RDebugMenu.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "Rhino.h"
#include "RDebugMenu.h"

RDebugMenu::RDebugMenu()
	: m_SelMenu(0), m_bEnabled(true)
{
}


RDebugMenu::~RDebugMenu()
{
}

void RDebugMenu::AddFloatMenuItem(const char* name, float* val, float step)
{
	MenuItem item;
	strcpy_s(item.name, 256, name);
	item.type = MenuItem::Item_Float;
	item.fval = val;
	item.fstep = step;

	m_MenuItems.push_back(item);
}

void RDebugMenu::AddIntMenuItem(const char* name, int* val, int step)
{
	MenuItem item;
	strcpy_s(item.name, 256, name);
	item.type = MenuItem::Item_Int;
	item.ival = val;
	item.istep = step;

	m_MenuItems.push_back(item);
}

void RDebugMenu::AddBoolMenuItem(const char* name, bool* val)
{
	MenuItem item;
	strcpy_s(item.name, 256, name);
	item.type = MenuItem::Item_Bool;
	item.bval = val;

	m_MenuItems.push_back(item);
}

void RDebugMenu::Update()
{
	if (!m_bEnabled)
		return;

	if (IsKeyDownOrRepeat(VK_DOWN))
	{
		m_SelMenu++;
		m_SelMenu %= m_MenuItems.size();
	}

	if (IsKeyDownOrRepeat(VK_UP))
	{
		if (m_SelMenu == 0)
			m_SelMenu = (UINT)m_MenuItems.size() - 1;
		else
			m_SelMenu--;
	}

	if (IsKeyDownOrRepeat(VK_RIGHT))
	{
		MenuItem& item = m_MenuItems[m_SelMenu];
		item.Increase();
	}

	if (IsKeyDownOrRepeat(VK_LEFT))
	{
		MenuItem& item = m_MenuItems[m_SelMenu];
		item.Decrease();
	}

	m_Vertices.clear();

	// Calculate longest menu item name for alignment
	int max_name_length = 0;
	for (UINT i = 0; i < m_MenuItems.size(); i++)
	{
		int len = (int)strlen(m_MenuItems[i].name);
		if (len > max_name_length)
			max_name_length = len;
	}

	for (UINT i = 0; i < m_MenuItems.size(); i++)
	{
		MenuItem& item = m_MenuItems[i];

		static char msg_buf[1024];
		char spacing[256] = { 0 };

		int space_count = max_name_length + 1 - (int)strlen(item.name);
		for (int j = 0; j < space_count; j++)
		{
			spacing[j] = ' ';
		}
		switch (item.type)
		{
		case MenuItem::Item_Float:
			sprintf_s(msg_buf, 1024, "%s%s: %f", item.name, spacing, item.fval ? *item.fval : 0.0f);
			break;
		case MenuItem::Item_Int:
			sprintf_s(msg_buf, 1024, "%s%s: %d", item.name, spacing, item.ival ? *item.ival : 0);
			break;
		case MenuItem::Item_Bool:
			sprintf_s(msg_buf, 1024, "%s%s: %s", item.name, spacing, (item.bval && *item.bval) ? "true" : "false");
			break;
		}
		AddText(msg_buf, 4, i + 2, RColor(1, 1, 1), RColor(0, 0, 0));
	}

	// Draw cursor for selected menu item
	AddText(">", 2, m_SelMenu + 2, RColor(1, 1, 1), RColor(0, 0, 0));
}

void RDebugMenu::Render()
{
	if (m_bEnabled)
		RText::Render();
}

bool RDebugMenu::IsKeyDownOrRepeat(int keycode)
{
	if (RInput.GetBufferedKeyState(keycode) == BKS_Pressed)
	{
		m_KeyRepeatTime = REngine::GetTimer().TotalTime() + 0.4f;
		return true;
	}
	
	if ((RInput.IsKeyDown(keycode) && m_KeyRepeatTime < REngine::GetTimer().TotalTime()))
	{
		m_KeyRepeatTime = REngine::GetTimer().TotalTime() + 0.01f;
		return true;
	}

	return false;
}

void RDebugMenu::MenuItem::Increase()
{
	switch (type)
	{
	case Item_Float:
		if (fval)
			*fval += fstep;
		break;
	case Item_Int:
		if (ival)
			*ival += istep;
		break;
	case Item_Bool:
		if (bval)
			*bval = !*bval;
		break;
	}
}

void RDebugMenu::MenuItem::Decrease()
{
	switch (type)
	{
	case Item_Float:
		if (fval)
			*fval -= fstep;
		break;
	case Item_Int:
		if (ival)
			*ival -= istep;
		break;
	case Item_Bool:
		if (bval)
			*bval = !*bval;
		break;
	}
}
