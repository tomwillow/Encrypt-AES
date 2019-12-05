#include <Windows.h>
#include <vector>
#include <string>
#include <thread>//多线程

//启用拖放需要
#include <shellapi.h> 
#pragma comment(lib, "shell32.lib")

#include "TStatic.h"
#include "TButton.h"
#include "TEdit.h"
#include "TDropEdit.h"
#include "TCheckBox.h"
#include "TProgress.h"
#include "TTabControl.h"

#include "resource.h"

#include "FileFunction.h"
#include "ProcessArgu.h"

#include "RSA/RSA.h"

//样式使用
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")
#include "ControlStyle.h"

//dpi
#include <ShellScalingAPI.h>
#pragma comment(lib,"Shcore.lib")

using namespace std;

#ifdef _WIN64
const TCHAR AppTitle[] = TEXT("文件加密器x64 v1.1");
#else
const TCHAR AppTitle[] = TEXT("文件加密器x32 v1.1");
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

void FreshEdit2(TCheckBox& CheckBox, TDropEdit& Edit2, TButton& BtnDialog2)
{
	static string fileName2;
	if (CheckBox.GetChecked())
	{
		if (!Edit2.GetText().empty())
			fileName2 = Edit2.GetText();
		Edit2.SetText(TEXT(""));
		Edit2.SetEnable(false);
		BtnDialog2.SetEnable(false);
	}
	else
	{
		if (!fileName2.empty())
			Edit2.SetText(fileName2);
		Edit2.SetEnable(true);
		BtnDialog2.SetEnable(true);
	}
}


INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static TDropEdit Edit1;
	static TDropEdit Edit2;
	static TEdit EditDES;
	static TDropEdit EditRSAPublic, EditRSAPrivate;
	static TCheckBox CheckBox;
	static TButton BtnGenRSAKey;
	static TButton BtnDialog1, BtnDialog2;
	static TButton BtnDialogRSA;
	static TButton BtnEncrypt, BtnDecrypt;
	static TProgress Progress;
	static TTabControl Tab;
	static TStatic StaticDES;
	static TStatic StaticRSAPublic;
	static TStatic StaticRSAPrivate;
	static const vector<pair<string, string>> vecFilter = { {"所有文件","*.*"} };
	static const vector<pair<string, string>> vecPubFilter = { {"公钥文件pubkey","*.pubkey"}, {"所有文件","*.*"} };
	static const vector<pair<string, string>> vecPriFilter = { {"私钥文件prikey","*.prikey"},{"所有文件","*.*"} };
	switch (message)
	{
	case WM_INITDIALOG:
	{
		SetWindowText(hDlg, AppTitle);

		//Edit
		Edit1.LinkControl(hDlg, IDC_EDIT1);
		Edit1.SetDragAccept(true);

		Edit2.LinkControl(hDlg, IDC_EDIT2);
		Edit2.SetDragAccept(true);

		EditDES.LinkControl(hDlg, IDC_EDIT_DES);

		EditRSAPublic.LinkControl(hDlg, IDC_EDIT_RSA);
		EditRSAPublic.SetDragAccept(true);

		EditRSAPrivate = EditRSAPublic;

		//CheckBox
		CheckBox.LinkControl(hDlg, IDC_CHECK);
		CheckBox.SetVisible(false);

		//Progress
		Progress.LinkControl(hDlg, IDC_PROGRESS);

		//button
		BtnGenRSAKey.LinkControl(hDlg, IDC_BTN_GENRSAKEY);
		BtnDialog1.LinkControl(hDlg, IDC_BTN_DIALOG_1);
		BtnDialog2.LinkControl(hDlg, IDC_BTN_DIALOG_2);

		BtnDialogRSA.LinkControl(hDlg, IDC_BTN_DIALOG_RSA);

		BtnEncrypt.LinkControl(hDlg, IDC_BTN_CONVERT);

		BtnDecrypt = BtnEncrypt;
		BtnDecrypt.SetText("解密");

		//static
		StaticDES.LinkControl(hDlg, IDC_STATIC_DES);

		StaticRSAPublic.LinkControl(hDlg, IDC_STATIC_RSA);

		StaticRSAPrivate = StaticRSAPublic;
		StaticRSAPrivate.SetText("RSA私钥：");

		Tab.LinkControl(hDlg, IDC_TAB);
		Tab.SetRectAsParent();

		Tab.AddTabItem("加密");
		Tab.AddTabItem("解密");

		Tab.TakeOverControl(0, { &StaticRSAPublic,&EditRSAPublic,&StaticDES,&EditDES,&BtnGenRSAKey,&BtnEncrypt });
		Tab.TakeOverControl(1, { &StaticRSAPrivate,&EditRSAPrivate,&BtnDecrypt });

		Tab.SetCurSel(0);

		//#ifdef _DEBUG
		EditRSAPublic.SetText("C:\\Users\\jiaoshou\\Desktop\\1.pubkey");
		EditRSAPrivate.SetText("C:\\Users\\jiaoshou\\Desktop\\2.prikey");
		Edit1.SetText("C:\\Users\\jiaoshou\\Desktop\\1.zip");
		Edit2.SetText("C:\\Users\\jiaoshou\\Desktop\\2.zip");
		EditDES.SetText("天下为公");
		//#endif
				//BtnGenRSAKey.RegisterMessage(WM_COMMAND,[]() {MessageBeep(0); });

		return TRUE;
	}
	case WM_NOTIFY:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BTN_GENRSAKEY:
		{
			try
			{
				if (MessageBox(hDlg, TEXT("即将生成密钥对，可能需要5秒至1分钟，确认生成吗？"), TEXT(""), MB_YESNO | MB_ICONINFORMATION) == IDNO)
					break;
				RSA rsa;
				rsa.generateKeyPair();

				string publicFileName, privateFileName;
				TFileDialog fd(hDlg);
				fd.SetFilter(vecPubFilter);
				fd.SetTitle("请选择公钥保存位置：");
				if (fd.Save(publicFileName))
				{
					rsa.savePublicKey(publicFileName.c_str());
					EditRSAPublic.SetText(publicFileName);

					fd.SetszFile("");
					fd.SetFilter(vecPriFilter);
					fd.SetTitle("请选择私钥保存位置：");
					if (fd.Save(privateFileName))
					{
						rsa.savePrivateKey(privateFileName.c_str());
						EditRSAPrivate.SetText(privateFileName);

						MessageBox(hDlg, TEXT("保存完成."), TEXT(""), MB_OK | MB_ICONINFORMATION);
					}
				}
			}//end of try
			catch (string & err)
			{
				MessageBox(hDlg, err.c_str(), TEXT("错误"), MB_OK | MB_ICONERROR);
			}
			break;
		}
		case IDC_BTN_DIALOG_RSA:
		{
			switch (Tab.GetCurSel())
			{
			case 0:
			{
				string pubFileName;
				TFileDialog fd(hDlg, vecPubFilter);
				if (fd.Open(pubFileName))
				{
					EditRSAPublic.SetText(pubFileName);
				}
				break;
			}
			case 1:
			{
				string priFileName;
				TFileDialog fd(hDlg, vecPriFilter);
				if (fd.Open(priFileName))
				{
					EditRSAPrivate.SetText(priFileName);
				}
				break;
			}
			}
			break;
		}
		case IDC_CHECK:
		{
			FreshEdit2(CheckBox, Edit2, BtnDialog2);
			break;
		}
		case IDC_BTN_DIALOG_1:
		{
			static TFileDialog fd(hDlg, vecFilter);
			string fileName1;
			if (fd.Open(fileName1))
			{
				Edit1.SetText(fileName1);
			}
			break;
		}
		case IDC_BTN_DIALOG_2:
		{
			static TFileDialog fd(hDlg, vecFilter);
			string fileName2;
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
				//取得输入输出文件名
				string fileName1 = Edit1.GetText();
				string fileName2;
				if (CheckBox.GetChecked())
					fileName2 = fileName1;
				else
					fileName2 = Edit2.GetText();

				//文件验证
				if (!GetFileExists(fileName1.c_str()))
					throw string("输入文件不存在");

				if (fileName2.empty())
					throw string("输出文件为空");

				if (fileName1 == fileName2)
					throw string("输入/输出文件不能相同");

				bool encrypt = Tab.GetCurSel() == 0;

				string desCipher;
				string keyFileName;
				if (encrypt)
				{
					//输入检测
					desCipher = EditDES.GetText();
					int len = desCipher.length();
					if (len == 0 || len > 8)
						throw string("DES密钥必须为1至8个字符");

					keyFileName = EditRSAPublic.GetText();
					if (!GetFileExists(keyFileName.c_str()))
						throw string("公钥文件不存在");
				}
				else
				{
					keyFileName = EditRSAPrivate.GetText();
					if (!GetFileExists(keyFileName.c_str()))
						throw string("私钥文件不存在");
				}

				ProcessArgu* pa = new ProcessArgu(
					AppTitle,
					encrypt,
					fileName1, fileName2, keyFileName,
					desCipher,
					hDlg, &Progress,
					&CheckBox, &Edit2, &BtnDialog2, encrypt ? &BtnEncrypt : &BtnDecrypt,

					{ &Edit1, &Edit2,&EditRSAPublic,&EditRSAPrivate,&EditDES,
					&BtnGenRSAKey,
					&BtnEncrypt,&BtnDecrypt,  &BtnDialog1, &BtnDialog2,&BtnDialogRSA,
					&CheckBox,&Tab });

				thread t(DealProc, pa);
				t.detach();

			}//end of try
			catch (string & err)
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