#include "TButton.h"

void TButton::CreateButton(HWND hParent,UINT uId,TCHAR text[],HINSTANCE hInst)
{
	m_hInst = hInst;
	m_hWnd = CreateWindow(
		TEXT("button"),//���������ĺ�ť����ʾ
		text,
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		0,
		0,
		0,
		0,
		hParent, (HMENU)uId,//id
		hInst,
		NULL);

	RegisterProc();
}