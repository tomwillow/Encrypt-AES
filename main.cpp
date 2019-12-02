#include <Windows.h>
#include <vector>
#include <string>
#include <memory> //智能指针
#include <process.h> //多线程

//启用拖放需要
#include <shellapi.h> 
#pragma comment(lib, "shell32.lib")

#include "TStatic.h"
#include "TButton.h"
#include "TEdit.h"
#include "TCheckBox.h"
#include "TProgress.h"
#include "TTabControl.h"
#include "resource.h"
#include "FileFunction.h"

#include "ScopeTime.h"
#include "RSA/RSA.h"
extern "C"
{
#include "DES/des.h"
}
//样式使用
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")
#include "ControlStyle.h"

//dpi
#include <ShellScalingAPI.h>
#pragma comment(lib,"Shcore.lib")

using namespace std;

const TCHAR AppTitle[] = TEXT("文件加密器");
const vector<pair<string, string>> vecFilter = { {"所有文件","*.*"} };
const vector<pair<string, string>> vecPubFilter = { {"公钥文件pubkey","*.pubkey"}, {"所有文件","*.*"} };
const vector<pair<string, string>> vecPriFilter = { {"私钥文件prikey","*.prikey"},{"所有文件","*.*"} };
HWND hDlg;
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


class TDropEdit :public TEdit
{
protected:
	void DropProc(const vector<string>& dropFiles)override
	{
		if (dropFiles.size() == 1)
			this->SetText(dropFiles[0]);
	}
};

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

class ProcessArgu
{
private:
	HWND hDlg;
	TProgress* pProgress;
	TCheckBox* pCheckBox;
	TDropEdit* pEdit2;
	TButton* pBtnDialog2, * pBtnConvert;
	string sBtnConvert;
	vector<TControl*> vechShouldDisable;
public:
	ScopeTime st;
	bool encrypt;
	string fileName1, fileName2, keyFileName;
	string desKey;
	FILE* fpp1, * fpp2;
	unsigned char* buf;
	bool success;
	ProcessArgu() = delete;
	ProcessArgu(const ProcessArgu&) = delete;
	ProcessArgu(bool encrypt, string fileName1, string fileName2, string keyFileName,
		string desKey,
		HWND hDlg, TProgress* pProgress,
		TCheckBox* pCheckBox, TDropEdit* pEdit2, TButton* pBtnDialog2, TButton* pBtnConvert,
		const initializer_list<TControl*>& vechShouldDisable) :

		encrypt(encrypt), fileName1(fileName1), fileName2(fileName2), keyFileName(keyFileName),
		desKey(desKey),
		hDlg(hDlg), pProgress(pProgress),
		pCheckBox(pCheckBox), pEdit2(pEdit2), pBtnDialog2(pBtnDialog2), pBtnConvert(pBtnConvert),
		vechShouldDisable(vechShouldDisable)
	{
		fpp1 = nullptr;
		fpp2 = nullptr;
		buf = nullptr;
		success = false;

		//全局控件禁用
		for (auto h : vechShouldDisable)
			h->SetEnable(false);

		sBtnConvert = pBtnConvert->GetText();

		pProgress->SetPos(0);
	}

	~ProcessArgu()
	{
		//释放资源
		if (fpp1)
			fclose(fpp1);
		if (fpp2)
			fclose(fpp2);
		if (buf)
			free(buf);

		//恢复控件状态
		for (auto h : vechShouldDisable)
			h->SetEnable(true);

		FreshEdit2(*pCheckBox, *pEdit2, *pBtnDialog2);

		pBtnConvert->SetText(sBtnConvert);

		//设置完成状态并弹窗
		if (success)
		{
			string caption = "处理完成。用时：" + st.elapsed();
			pProgress->SetPos(100);
			MessageBox(hDlg, caption.c_str(), TEXT(""), MB_OK | MB_ICONINFORMATION);

			SetWindowText(hDlg, AppTitle);
		}
	}

	//更新进度条
	void SetProgress(int per,double velocityKBperS,double remainSecond)
	{
		pProgress->SetPos(per);
		string sPercent = to_string(per) + "%";
		pBtnConvert->SetText(sPercent);

		TCHAR title[MAX_PATH];
		sprintf_s(title,MAX_PATH, "%s 速度:%.0f KB/s 当前:%d%% 预计剩余:%.2f s", AppTitle,velocityKBperS,per,remainSecond);
		SetWindowText(hDlg, title);
	};
};

void DealProc(void* p)try
{
	//将p移交智能指针管理
	shared_ptr<ProcessArgu> sp((ProcessArgu*)p);
	if (fopen_s(&sp->fpp1, sp->fileName1.c_str(), "rb+"))
		throw string("打开") + sp->fileName1 + string("文件出错，检查文件是否被占用或者没有权限");

	sp->SetProgress(0,0.0,-1);

	RSA rsa;
	if (sp->encrypt)
	{
		rsa.readPublicKey(sp->keyFileName.c_str());
	}
	else
	{
		rsa.readPrivateKey(sp->keyFileName.c_str());
	}

	//得到文件大小
	fseek(sp->fpp1, 0L, SEEK_END);
	long fileSize = ftell(sp->fpp1);
	fseek(sp->fpp1, 0L, SEEK_SET);

	//缓冲区大小 若缓冲区大于文件大小，后续将进行修正
	long bufSize = 0x00010000;//1MB 0x00100000

	//if (fileSize > 0x6400000)//100MB
	//	bufSize = fileSize / 50;

	sp->buf = (unsigned char*)malloc(bufSize);
	unsigned char* buf = sp->buf; //由sp负责释放
	if (buf == nullptr)
		throw string("分配缓冲区内存失败，请关闭部分应用再试");

	//处理位置
	long cur = 0;
	int percent = 0;
	int prev = 0;
	unsigned char remain_buf[8];
	size_t remain_len = 0;

	//开始读写
	if (fopen_s(&sp->fpp2, sp->fileName2.c_str(), "wb"))
		throw string("打开") + sp->fileName2 + string("文件出错，检查文件是否被占用或者没有权限");


	FILE* fp1 = sp->fpp1;
	FILE* fp2 = sp->fpp2;
	long readed = 0;

	//得到DES密钥
	uint64_t desKey = GetDESKeyByC(sp->desKey.c_str(), sp->desKey.length());
	if (sp->encrypt)
	{
		//加密
		//密文文件格式：[RSA编码块大小][RSA加密后的DES密钥][原文件大小][密文]
		//                  4B                  ~84B            4B
		int maxBlockBytes = rsa.getMaxBlockBytes();

		unsigned char* encryptedDesKey = buf;
		memset(encryptedDesKey, 0, maxBlockBytes);
		//密钥明文存入缓存
		WriteDESKey(desKey, encryptedDesKey);

		//写入RSA编码块大小
		int encodedBlockSize = rsa.getEncodeBlockSize();
		fwrite(&encodedBlockSize, sizeof(encodedBlockSize), 1, fp2);

		//写入RSA密文
		rsa.encodeToFile(fp2, maxBlockBytes, encryptedDesKey);

		//写入原文件大小
		fwrite(&fileSize, sizeof(fileSize), 1, fp2);

		//下面可以写入密文
	}
	else
	{
		//解密
		//读出RSA编码块大小
		int encodedBlockSize = 0;
		fread(&encodedBlockSize, sizeof(encodedBlockSize), 1, fp1);
		if (encodedBlockSize > fileSize)
			throw string("文件格式错误：RSA编码块大于文件大小");

		//读出RSA密文
		unsigned char* encryptedDesKey = buf;
		fread(encryptedDesKey, encodedBlockSize, 1, fp1);

		rsa.decode(encryptedDesKey, encryptedDesKey, encodedBlockSize, encodedBlockSize);

		desKey = GetDESKeyByUC(encryptedDesKey, 8);

		//读出原文件大小
		long realSize = 0;
		readed = fread(&realSize, sizeof(realSize), 1, fp1);
		fileSize = realSize;

		//可以开始解密
	}

	while (1)
	{
		//如果文件剩余部分小于分块，则将分块设为剩余大小
		bufSize = min(bufSize, fileSize - cur);

		//本次读取字节数
		readed = fread(buf, bufSize, 1, fp1) * bufSize;

		//无读取说明已读完
		if (readed == 0)
			break;

		//处理
		if (sp->encrypt)//加密
			EncryptData(buf, bufSize, remain_buf, &remain_len, desKey);
		else//解密
			DecryptData(buf, bufSize, desKey);

		//写入
		fwrite(buf, readed, 1, fp2);

		if (remain_len)
			fwrite(remain_buf, remain_len, 1, fp2);

		//更新已读取数量
		cur += readed;

		//更新状态
		percent = 100LL * cur / fileSize;
		if (sp->st.flew1sec())
		//if (percent > prev) //百分比发生变化再更新
		{
			double velocityKBperS = (cur) / 1024.0 / (sp->st.elapsedMilliseconds() / 1000.0);
			double remainSecond = (fileSize - cur) / 1024.0 / velocityKBperS;
			sp->SetProgress(percent,velocityKBperS,remainSecond);
			prev = percent;
		}
	}

	//设置成功标记，sp将复位控件
	sp->success = true;

	//文件句柄由sp释放

	//sp将自动释放
}
catch (string & err)
{
	MessageBox(hDlg, err.c_str(), TEXT("错误"), MB_OK | MB_ICONERROR);
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
	switch (message)
	{
	case WM_INITDIALOG:
	{
		::hDlg = hDlg;
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
					encrypt,
					fileName1, fileName2, keyFileName,
					desCipher,
					hDlg, &Progress,
					&CheckBox, &Edit2, &BtnDialog2, encrypt ? &BtnEncrypt : &BtnDecrypt,

					{ &Edit1, &Edit2,&EditRSAPublic,&EditRSAPrivate,&EditDES,
					&BtnGenRSAKey,
					&BtnEncrypt,&BtnDecrypt,  &BtnDialog1, &BtnDialog2,&BtnDialogRSA,
					&CheckBox,&Tab });

				_beginthread(DealProc, 0, pa);

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