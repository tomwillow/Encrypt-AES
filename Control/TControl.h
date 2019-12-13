#pragma once
#include <Windows.h>
#include <vector>
#include <unordered_map>
#include "tstring.h"

class TControl
{
private:
	bool bCanAcceptDrag;
	std::unordered_map<UINT, void(*)(void)> msgDealer;
	HFONT m_hFont;
	static LRESULT CALLBACK subControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	std::vector<tstring> PreDrop(WPARAM wParam) const;
public:
	//LONG m_iWidth, m_iHeight;
	TControl();
	virtual ~TControl();

	TControl& operator=(const TControl& control);

	HWND GetHWND();

	void LinkControl(HWND hwnd);
	void LinkControl(HWND hDlg, int id);

	RECT GetClientRect() const;
	RECT GetWindowRect() const;

	void SetRect(RECT &rect);
	void SetRect(int x1, int y1, int x2, int y2); 
	//void SetPos(RECT &rect);
	void Invalidate();

	void SetFont(HFONT hFont);
	void SetDefaultGuiFont();
	void SetFont(TCHAR FontName[], int FontSize);

	void SetText(const tstring &s);
	void CDECL SetText(const TCHAR szFormat[], ...);//��������
	void GetText(TCHAR text[]);
	TCHAR* GetTCHAR();//����ֵ��TControl�Լ������ͷ�
	tstring GetText();
	int GetLength();//��ȡ�ַ�������	

	RECT GetPosition() const;
	void SetPosition(int x, int y, int width, int height);//���ô�С��λ��
	void SetPosition(RECT rect);//���ô�С��λ��
	void SetPositionOnlyOrigin(const RECT &rect);

	void SetVisible(bool bShow);//���ÿɼ���
	bool GetVisible();

	void SetDouble(double d);
	double GetDouble();

	void SetEnable(bool bEnable);
	bool GetEnable();

	void SetID(int id);

	void SetDragAccept(bool bCanAcceptDrop);

	void RegisterMessage(UINT uMsg,void(*fun)(void));
protected:
	HWND m_hParent;
	HWND m_hWnd;
	HINSTANCE m_hInst;
	TCHAR *Text;
	void RegisterProc();//�������ں�ע��

	virtual LRESULT WndProc(WNDPROC wndproc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//������Ϣ���������ɸ���

	virtual void DropProc(const std::vector<tstring>& dropFiles);
};
