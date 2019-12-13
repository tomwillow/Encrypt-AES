#pragma once

#include "tchar_head.h"
#include <stdio.h>//_vsnprintf_s
#include <vector>
#include "TControl.h"

#include "TTransfer.h"

using namespace std;

TControl::TControl()
{
	m_hWnd = NULL;
	m_hParent = NULL;
	m_hInst = NULL;

	Text = NULL;

	m_hFont = NULL;

	bCanAcceptDrag = false;
}


TControl::~TControl()
{
	if (Text != NULL)
		free(Text);

	if (m_hFont!=NULL)
	::DeleteObject(m_hFont);
}

TControl& TControl::operator=(const TControl& control)
{
	TCHAR className[MAX_PATH];
	GetClassName(control.m_hWnd, className, MAX_PATH);

	LONG style = GetWindowLong(control.m_hWnd, GWL_STYLE);
	LONG exstyle = GetWindowLong(control.m_hWnd, GWL_EXSTYLE);
	LONG id = GetWindowLong(control.m_hWnd, GWL_ID);

	RECT rc = control.GetClientRect();
	RECT rcPos = control.GetPosition();

	m_hParent = control.m_hParent;
	m_hInst = control.m_hInst;
	m_hWnd = CreateWindowEx(exstyle,className, GetTCHAR(),
		style,rcPos.left,rcPos.top, rc.right, rc.bottom, 
		control.m_hParent, (HMENU)id, control.m_hInst, 0);
	
	HFONT hFont = (HFONT)SendMessage(control.m_hWnd, WM_GETFONT, 0, 0);
	SetFont(hFont);

	SetDragAccept(control.bCanAcceptDrag);
	return *this;
}


//��ʹ��x,y���꣬width,heightʹ��ԭ��С
void TControl::SetPositionOnlyOrigin(const RECT &rect)
{
	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	SetPosition(rect.left, rect.top, rc.right, rc.bottom);
}

//right��bottom������ǿ�͸�
void TControl::SetRect(RECT &rect)
{
	::MoveWindow(m_hWnd, rect.left,rect.top,  rect.right, rect.bottom, true);
}

//�Խǵ�����
void TControl::SetRect(int x1, int y1, int x2, int y2)
{
	::MoveWindow(m_hWnd, x1, y1, x2 - x1, y2 - y1, true);
}

//���أ�����ڸ����ڵ� x1, y1, x2, y2
RECT TControl::GetPosition() const
{
	RECT rc = GetWindowRect();
	POINT pt1 = { rc.left,rc.top };
	POINT pt2 = { rc.right,rc.bottom };
	ScreenToClient(m_hParent, &pt1);
	ScreenToClient(m_hParent, &pt2);
	return { pt1.x,pt1.y,pt2.x,pt2.y };
}

void TControl::SetPosition(int x, int y, int width, int height)
{
	::MoveWindow(m_hWnd, x, y, width, height, true);
	//::SetWindowPos(m_hWnd, HWND_TOP, x, y, width, height, 0);//SWP_SHOWWINDOW
}

//rect�и�ֵ��Ϊ����
void TControl::SetPosition(RECT rect)
{
	::MoveWindow(m_hWnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
	//::SetWindowPos(m_hWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, 0);//SWP_SHOWWINDOW
}

vector<tstring> TControl::PreDrop(WPARAM wParam) const
{
	vector<tstring> ret;
	HDROP hDrop = (HDROP)wParam;
	UINT nFileNum = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); // ��ק�ļ�����
	TCHAR strFileName[MAX_PATH];
	for (int i = 0; i < nFileNum; i++)
	{
		DragQueryFile(hDrop, i, strFileName, MAX_PATH);//�����ҷ���ļ���
		ret.push_back(strFileName);
	}
	DragFinish(hDrop);      //�ͷ�hDrop
	return ret;
}

void TControl::DropProc(const std::vector<tstring>& dropFiles)
{

}

LRESULT CALLBACK TControl::subControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TControl * pControl;
#ifdef _WIN64
	pControl = (TControl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
	pControl = (TControl *)GetWindowLong(hWnd, GWL_USERDATA);
#endif

	WNDPROC oldProc;
	oldProc = (WNDPROC)GetProp(hWnd, TEXT("oldProc"));
	if (pControl)
	{
		if (uMsg == WM_DROPFILES)
		{
			vector<tstring> dropFiles=pControl->PreDrop(wParam);
			pControl->DropProc(dropFiles);
		}
		else
			return pControl->WndProc((WNDPROC)oldProc, hWnd, uMsg, wParam, lParam);
	}
	else
		return CallWindowProc((WNDPROC)oldProc, hWnd, uMsg, wParam, lParam);
}

void TControl::LinkControl(HWND hDlg, int id)
{
	LinkControl(GetDlgItem(hDlg, id));
}

void TControl::LinkControl(HWND hwndControl)//���ӵ����пؼ������ڶԻ����У�
{
	m_hInst = GetModuleHandle(NULL);
	m_hParent = GetParent(hwndControl);
	m_hWnd = hwndControl;

	//���Է���subControlProc�лᷴ������ uMsg=0x87, WM_GETDLGCODE ��Ϣ��
	//��ѯ https://blog.csdn.net/amwfnyq/article/details/5612289 �����ǿؼ�
	//û�� WS_EX_CONTROLPARENT ��ʽ������ѭ��
	//LONG style = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	//SetWindowLong(m_hWnd, GWL_EXSTYLE, style | WS_EX_CONTROLPARENT);

	//�ٴβ��Է���������HWNDע�ᵽͬһ��class������ѭ��

	RegisterProc();
}

void TControl::RegisterProc()//�������ں�ע��
{
#ifdef _WIN64
	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
	WNDPROC oldProc = (WNDPROC)::SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)subControlProc);
#else
	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
	WNDPROC oldProc = (WNDPROC)::SetWindowLongPtr(m_hWnd, GWL_WNDPROC, (LONG_PTR)subControlProc);
#endif
	
	SetProp(m_hWnd, TEXT("oldProc"), oldProc);
}

LRESULT TControl::WndProc(WNDPROC wndproc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)//������Ϣ��������������
{
	auto it = msgDealer.find(uMsg);
	if (it != msgDealer.end())
	{
		it->second();
	}
	return CallWindowProc(wndproc, hWnd, uMsg, wParam, lParam);
}

HWND TControl::GetHWND()
{
	return m_hWnd;
}

void TControl::Invalidate()
{
	InvalidateRect(m_hWnd, &GetClientRect(), FALSE);
}

void TControl::SetFont(HFONT hFont)
{
	SendMessage(m_hWnd,             // Handle of edit control
		WM_SETFONT,         // Message to change the font
		(WPARAM)hFont,     // handle of the font
		MAKELPARAM(TRUE, 0) // Redraw text
		);
}

void TControl::SetDefaultGuiFont()
{
	if (m_hFont != NULL)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}

	m_hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);//
	SetFont(m_hFont);
}

void TControl::SetFont(TCHAR FontName[], int FontSize)
{
	if (m_hFont != NULL)
	{
		DeleteObject(m_hFont);
		m_hFont = NULL;
	}
	
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(lf));
	_tcscpy_s(lf.lfFaceName,_tcslen(FontName)+1, FontName);
	lf.lfHeight = -MulDiv(FontSize, GetDeviceCaps(GetDC(m_hWnd), LOGPIXELSY), 72);
	//lf.lfWeight = 900;
	
	m_hFont = CreateFontIndirect(&lf);
	SetFont(m_hFont);
}

void TControl::SetText(const tstring &s)
{
	::SetWindowText(m_hWnd, s.c_str());
}

void CDECL TControl::SetText(const TCHAR szFormat[], ...)
{
	TCHAR szBuffer[1024];
	va_list pArgList;
	va_start(pArgList, szFormat);
	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(TCHAR), szFormat, pArgList);
	va_end(pArgList);

	::SetWindowText(m_hWnd, szBuffer);
}


void TControl::GetText(TCHAR text[])
{
	::GetWindowText(m_hWnd, text, GetLength() + 1);//��֪��ΪʲôҪ��1��ȡ��ȫ
}

tstring TControl::GetText()
{
	return GetTCHAR();
}

TCHAR * TControl::GetTCHAR()
{
	Text = (TCHAR *)realloc(Text, (GetLength() + 1)*sizeof(TCHAR));
	GetText(Text);
	return Text;
}

int TControl::GetLength()
{
	return ::GetWindowTextLength(m_hWnd);
}

void TControl::SetVisible(bool bShow)
{
	::ShowWindow(m_hWnd, bShow ? SW_SHOWNORMAL : SW_HIDE);
}

bool TControl::GetVisible()
{
	return (bool)IsWindowVisible(m_hWnd);
}

void TControl::SetEnable(bool bEnable)
{
	EnableWindow(m_hWnd, bEnable);
}

bool TControl::GetEnable()
{
	return (bool)IsWindowEnabled(m_hWnd);
}

//�Զ�ȥ��С��ĩβ0�������ʾ6λ
void TControl::SetDouble(double d)
{
	TCHAR s[64];
	TTransfer::double2TCHAR_AutoTrim0(d, s);
	SetText(s);
}

double TControl::GetDouble()
{
	return _tcstod(GetTCHAR(), NULL);
}

//��ù�������С
RECT TControl::GetClientRect() const
{
	RECT rect;
	::GetClientRect(m_hWnd, &rect);
	return rect;
}

RECT TControl::GetWindowRect()const
{
	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	return rect;
}

void TControl::SetDragAccept(bool bCanAcceptDrop)
{
	this->bCanAcceptDrag = bCanAcceptDrop;
	DragAcceptFiles(m_hWnd, bCanAcceptDrop);
}

void TControl::RegisterMessage(UINT uMsg,void(*fun)(void))
{
	msgDealer[uMsg] = fun;
}

void TControl::SetID(int id)
{
	SetWindowLong(m_hWnd, GWL_ID, id);
}