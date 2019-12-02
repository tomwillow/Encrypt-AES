#pragma once
#include <Windows.h>

class TWindow
{
private:
	bool m_bMainWindow;
	HICON m_hTitleIcon;//ͼ��
	HACCEL m_hAccelTable;//��ݼ�
	bool m_bDoubleBuffer;//˫����
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//��̬��Ϣ����
protected:

	virtual void OnDraw(HDC hdc)	{	}
	virtual void OnKeyDown(WPARAM wParam, LPARAM lParam)	{	}
	virtual void OnCommand(WPARAM wParam, LPARAM lParam){	}
	virtual void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam){	}//���ش��¼�ʱ����ʹ��m_hWnd��Ϊ���ھ��
	virtual void OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){	}
	virtual void OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){ }
	virtual void OnRButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){	}
	virtual void OnRButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){	}
	virtual void OnHotKey(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){	}

	virtual void OnMButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){	}
	virtual void OnMButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){	}
	virtual void OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){	}
	virtual void OnMouseWheel(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){	}

	virtual void OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){ }
	virtual bool OnClose(){ return true;}
	virtual void OnSize(WPARAM wParam, LPARAM lParam){	}
	virtual void OnSetCursor(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		//WM_SETCURSOR�¼���׽��ϵͳ����������仯�����������������ã���꽫��ɡ�æ����ꡣ
		DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
    
	virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//��Ϣ����

	virtual void GetWndClassEx(WNDCLASSEX & wc);

public:
	TCHAR *szName;//����
	HWND m_hWnd;
	HWND m_hParent;
	HINSTANCE m_hInst;
	RECT ClientRect;//��WM_PAINT���Զ�����
	RECT WindowRect;
	TWindow(void)
	{
		m_bMainWindow = false;
		m_hParent = NULL;
		m_hWnd = NULL;
		m_bDoubleBuffer = false;
		m_hTitleIcon = NULL;
		szName = NULL;
	}

	virtual ~TWindow(void)
	{
		if (szName != NULL)
			delete[] szName;
	}

	void LoadTitleIcon(HINSTANCE hInst, UINT id);//���ڴ�������ͼ��
	virtual bool CreateEx(DWORD dwExStyle, LPCTSTR lpszClass, LPCTSTR lpszName, DWORD dwStyle, 
		int x, int y, int nWidth, int nHeight, HWND hParent, HMENU hMenu, HINSTANCE hInst);

	bool RegisterClass(LPCTSTR lpszClass, HINSTANCE hInst);

	virtual WPARAM MessageLoop();
	void SetAccel(UINT id);
	void SetDoubleBuffer(bool bDoubleBuffer);
	void CDECL SetText(TCHAR szFormat[], ...);
	void Invalidate();//�ػ洰��
	BOOL ShowWindow(int nCmdShow) const
	{
		return ::ShowWindow(m_hWnd, nCmdShow);
	}

	BOOL UpdateWindow(void) const
	{
		return ::UpdateWindow(m_hWnd);
	}

	//Rect�����߶�
	void SetRect(RECT rect)
	{
		::MoveWindow(m_hWnd, rect.left, rect.top, rect.right, rect.bottom, true);
	}

	//Rect��������
	void SetWindowRect(RECT &rect)
	{
		::MoveWindow(m_hWnd, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, true);
	}

	void SetRect(int x1, int y1, int x2, int y2)
	{
		::MoveWindow(m_hWnd, x1, y1, x2 - x1, y2 - y1, true);
	}
};


//void ShowMessage(const TCHAR szFormat[], ...);
//int MyMessageBox(HWND hWnd, const TCHAR * text, const TCHAR * caption, DWORD style, int iconid);
