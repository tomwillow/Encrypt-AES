#include "TComboBox.h"

#include <windowsx.h>

using namespace std;

void TComboBox::AddItem(const tstring s, int value)
{
	ComboBox_AddString(m_hWnd, s.c_str());
	items.push_back(value);
}

int TComboBox::GetCurSelData()
{
	return items[ComboBox_GetCurSel(m_hWnd)];
}

void TComboBox::SetCurSel(int i)
{
	ComboBox_SetCurSel(m_hWnd, i);
}