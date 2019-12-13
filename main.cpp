#include <Windows.h>
#include <vector>
#include "tstring.h"
#include <thread>//多线程

//启用拖放需要
#include <shellapi.h> 
#pragma comment(lib, "shell32.lib")

#include "TStatic.h"
#include "TComboBox.h"
#include "TButton.h"
#include "TEdit.h"
#include "TDropEdit.h"
#include "TCheckBox.h"
#include "TProgress.h"
#include "TTabControl.h"

#include "resource.h"

#include "FileFunction.h"
#include "ProcessArgu.h"

#include "MyAES.h"

//样式使用
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")
#include "ControlStyle.h"

//dpi
#include <ShellScalingAPI.h>
#pragma comment(lib,"Shcore.lib")

using namespace std;

#ifdef _WIN64
const TCHAR AppTitle[] = TEXT("AES加密器x64 v1.0");
#else
const TCHAR AppTitle[] = TEXT("AES加密器x32 v1.0");
#endif

HINSTANCE hInst;

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	hInst = hInstance;
	InitCommonControls();
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
	if (-1 == DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, DlgProc))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"),
			TEXT(""), MB_ICONERROR);
	}
	return 0;
}


INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static TTabControl Tab;
	static TComboBox ComboBoxBits, ComboBoxMode;
	static TCheckBox CheckBoxQuick;
	static TEdit EditKey;
	static TEdit EditIV;
	static TDropEdit Edit1;
	static TDropEdit Edit2;
	static TButton BtnDialog1, BtnDialog2;
	static TButton BtnEncrypt, BtnDecrypt;
	static TProgress Progress;
	static const vector<pair<string, string>> vecFilter = { {"所有文件","*.*"} };
	switch (message)
	{
	case WM_INITDIALOG:
	{
		SetWindowText(hDlg, AppTitle);

		//ComboBox
		ComboBoxBits.LinkControl(hDlg, IDC_COMBO_BITS);
		ComboBoxBits.AddItem(TEXT("128位"), 128);
		ComboBoxBits.AddItem(TEXT("192位"), 192);
		ComboBoxBits.AddItem(TEXT("256位"), 256);
		ComboBoxBits.SetCurSel(0);

		ComboBoxMode.LinkControl(hDlg, IDC_COMBO_MODE);
		ComboBoxMode.AddItem(TEXT("ECB"), MyAES::ECB);
		ComboBoxMode.AddItem(TEXT("CBC"), MyAES::CBC);
		ComboBoxMode.AddItem(TEXT("CTR"), MyAES::CTR);
		ComboBoxMode.AddItem(TEXT("CFB-1"), MyAES::CFB1);
		ComboBoxMode.AddItem(TEXT("CFB-8"), MyAES::CFB8);
		ComboBoxMode.AddItem(TEXT("OFB-1"), MyAES::OFB1);
		ComboBoxMode.AddItem(TEXT("OFB-8"), MyAES::OFB8);
		ComboBoxMode.SetCurSel(0);

		//tab
		Tab.LinkControl(hDlg, IDC_TAB);
		Tab.SetRectAsParent();

		Tab.AddTabItem(TEXT("加密"));
		Tab.AddTabItem(TEXT("解密"));

		Tab.TakeOverControl(0, { &BtnEncrypt });
		Tab.TakeOverControl(1, { &BtnDecrypt });

		Tab.SetCurSel(0);

		//CheckBox
		CheckBoxQuick.LinkControl(hDlg, IDC_CHECK_QUICK);

		//Edit
		EditKey.LinkControl(hDlg, IDC_EDIT_KEY);

		EditIV.LinkControl(hDlg, IDC_EDIT_IV);

		Edit1.LinkControl(hDlg, IDC_EDIT1);
		Edit1.SetDragAccept(true);

		Edit2.LinkControl(hDlg, IDC_EDIT2);
		Edit2.SetDragAccept(true);

		//button
		BtnDialog1.LinkControl(hDlg, IDC_BTN_DIALOG_1);
		BtnDialog2.LinkControl(hDlg, IDC_BTN_DIALOG_2);

		//Progress
		Progress.LinkControl(hDlg, IDC_PROGRESS);


		BtnEncrypt.LinkControl(hDlg, IDC_BTN_CONVERT);
		BtnDecrypt = BtnEncrypt;
		BtnDecrypt.SetText(TEXT("解密"));

		#ifdef _DEBUG
		Edit1.SetText(TEXT("C:\\Users\\jiaoshou\\Desktop\\1.zip"));
		Edit2.SetText(TEXT("C:\\Users\\jiaoshou\\Desktop\\2.zip"));
		EditKey.SetText(TEXT("天下为公"));
		EditIV.SetText(TEXT("丝竹管弦"));
		#endif

		return TRUE;
	}
	case WM_NOTIFY:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_DIALOG_1:
		{
			static TFileDialog fd(hDlg, vecFilter);
			tstring fileName1;
			if (fd.Open(fileName1))
			{
				Edit1.SetText(fileName1);
			}
			break;
		}
		case IDC_BTN_DIALOG_2:
		{
			static TFileDialog fd(hDlg, vecFilter);
			tstring fileName2;
			if (fd.Save(fileName2))
			{
				Edit2.SetText(fileName2);
			}

			break;
		}
		case IDC_BTN_CONVERT:
		{
			try
			{
				bool encrypt = Tab.GetCurSel() == 0;

				int bits = ComboBoxBits.GetCurSelData();
				int mode = ComboBoxMode.GetCurSelData();
				bool quick = CheckBoxQuick.GetChecked();

				//输入检测
				tstring key = EditKey.GetText();
				int key_len = key.length();
				key_len = key.size();
				if (key_len == 0 || key_len > bits/8)
					throw tto_string(bits)+tstring(TEXT("位AES密钥必须为1至")) + tto_string(bits / 8) + tstring(TEXT("个字符"));

				//文件验证
				tstring fileName1 = Edit1.GetText();
				if (!GetFileExists(fileName1.c_str()))
					throw tstring(TEXT("输入文件不存在"));

				tstring fileName2 = Edit2.GetText();
				if (fileName2.empty())
					throw tstring(TEXT("输出文件不能为空"));

				if (fileName1 == fileName2)
					throw tstring(TEXT("输入/输出文件不能相同"));



				//ProcessArgu* pa = new ProcessArgu(
				//	AppTitle,
				//	encrypt,
				//	fileName1, fileName2, keyFileName,
				//	desCipher,
				//	hDlg, &Progress,
				//	&CheckBox, &Edit2, &BtnDialog2, encrypt ? &BtnEncrypt : &BtnDecrypt,

				//	{ &Edit1, &Edit2,&EditRSAPublic,&EditRSAPrivate,&EditDES,
				//	&BtnGenRSAKey,
				//	&BtnEncrypt,&BtnDecrypt,  &BtnDialog1, &BtnDialog2,&BtnDialogRSA,
				//	&CheckBox,&Tab });

				//thread t(DealProc, pa);
				//t.detach();

			}//end of try
			catch (tstring & err)
			{
				MessageBox(hDlg, err.c_str(), TEXT("错误"), MB_OK | MB_ICONERROR);
			}
			break;
		}
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}