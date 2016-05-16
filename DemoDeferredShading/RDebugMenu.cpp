//=============================================================================
// RDebugMenu.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RDebugMenu.h"

RDebugMenu::RDebugMenu()
	: m_SelMenu(0), m_bEnabled(true)
{
}


RDebugMenu::~RDebugMenu()
{
}

void RDebugMenu::AddMenuItem(const char* name, float* val, float step)
{
	MenuItem item;
	strcpy_s(item.name, 256, name);
	item.val = val;
	item.step = step;

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
			m_SelMenu = m_MenuItems.size() - 1;
		else
			m_SelMenu--;
	}

	if (IsKeyDownOrRepeat(VK_RIGHT))
	{
		MenuItem& item = m_MenuItems[m_SelMenu];
		if (item.val)
			*item.val += item.step;
	}

	if (IsKeyDownOrRepeat(VK_LEFT))
	{
		MenuItem& item = m_MenuItems[m_SelMenu];
		if (item.val)
			*item.val -= item.step;
	}

	m_Vertices.clear();

	int max_name_length = 0;
	for (UINT i = 0; i < m_MenuItems.size(); i++)
	{
		int len = strlen(m_MenuItems[i].name);
		if (len > max_name_length)
			max_name_length = len;
	}

	for (UINT i = 0; i < m_MenuItems.size(); i++)
	{
		MenuItem& item = m_MenuItems[i];
		if (m_SelMenu == i)
			AddText(">", 2, i + 2, RColor(1, 1, 1), RColor(0, 0, 0));
		char msg_buf[1024];
		char spacing[256] = { 0 };

		int space_count = max_name_length + 2 - strlen(item.name);
		for (int j = 0; j < space_count; j++)
		{
			spacing[j] = ' ';
		}
		sprintf_s(msg_buf, 1024, "%s%s: %f", item.name, spacing, item.val ? *item.val : 0.0f);
		AddText(msg_buf, 4, i + 2, RColor(1, 1, 1), RColor(0, 0, 0));
	}
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
		m_KeyRepeatTime = REngine::GetTimer().TotalTime() + 0.05f;
		return true;
	}

	return false;
}
