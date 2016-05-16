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

	if (RInput.GetBufferedKeyState(VK_DOWN) == BKS_Pressed)
	{
		m_SelMenu++;
		m_SelMenu %= m_MenuItems.size();
	}

	if (RInput.GetBufferedKeyState(VK_UP) == BKS_Pressed)
	{
		if (m_SelMenu == 0)
			m_SelMenu = m_MenuItems.size() - 1;
		else
			m_SelMenu--;
	}

	if (RInput.GetBufferedKeyState(VK_RIGHT) == BKS_Pressed)
	{
		MenuItem& item = m_MenuItems[m_SelMenu];
		if (item.val)
			*item.val += item.step;
	}

	if (RInput.GetBufferedKeyState(VK_LEFT) == BKS_Pressed)
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
