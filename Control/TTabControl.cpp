#include "TTabControl.h"

#include <algorithm>
#include <CommCtrl.h>

using namespace std;

TTabControl::~TTabControl()
{
}

void TTabControl::SetRectAsParent()
{
	RECT rc;
	::GetClientRect(m_hParent, &rc);
	SetRect(rc);
}

void TTabControl::AddTabItem(tstring label, std::initializer_list<TControl*> vecp)
{
	TCITEM tabItem;
	ZeroMemory(&tabItem, sizeof(TCITEM));
	tabItem.mask = TCIF_TEXT;

	TCHAR* text = new TCHAR[label.length() + 1];
	_tcscpy_s(text,label.length()+1, label.c_str());

	tabItem.pszText = text;

	TabCtrl_InsertItem(m_hWnd, pCtrls.size(), &tabItem);

	delete[] text;

	TakeOverControl(pCtrls.size(), vecp);
}

void TTabControl::TakeOverControl(int page,std::initializer_list<TControl*> vecp)
{
	if (page+1 >= pCtrls.size())
		pCtrls.resize(page+1);

	pCtrls[page].insert(vecp);
	pAllCtrls.insert(vecp);
}

int TTabControl::GetCurSel()
{
	return TabCtrl_GetCurSel(m_hWnd);
}

void TTabControl::SetCurSel(int sel)
{
	TabCtrl_SetCurSel(m_hWnd, sel);

	for_each(pAllCtrls.begin(), pAllCtrls.end(), [&](TControl* p) {p->SetVisible(false); });
	for_each(pCtrls[sel].begin(), pCtrls[sel].end(), [&](TControl* p) {p->SetVisible(true); });
}

LRESULT TTabControl::WndProc(WNDPROC wndproc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONUP:
	case WM_KEYUP:
		int sel =GetCurSel();

		for_each(pAllCtrls.begin(), pAllCtrls.end(), [&](TControl* p) {p->SetVisible(false); });
		for_each(pCtrls[sel].begin(), pCtrls[sel].end(), [&](TControl *p) {p->SetVisible(true); });
		break;
	}
	return CallWindowProc(wndproc, hWnd, uMsg, wParam, lParam);
}