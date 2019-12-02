#include <Windows.h>
#include <vector>
#include <string>
#include <memory> //����ָ��
#include <process.h> //���߳�

//�����Ϸ���Ҫ
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
//��ʽʹ��
#include <commctrl.h>
#pragma comment(lib,"comctl32.lib")
#include "ControlStyle.h"

//dpi
#include <ShellScalingAPI.h>
#pragma comment(lib,"Shcore.lib")

using namespace std;

const TCHAR AppTitle[] = TEXT("�ļ�������");
const vector<pair<string, string>> vecFilter = { {"�����ļ�","*.*"} };
const vector<pair<string, string>> vecPubFilter = { {"��Կ�ļ�pubkey","*.pubkey"}, {"�����ļ�","*.*"} };
const vector<pair<string, string>> vecPriFilter = { {"˽Կ�ļ�prikey","*.prikey"},{"�����ļ�","*.*"} };
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

		//ȫ�ֿؼ�����
		for (auto h : vechShouldDisable)
			h->SetEnable(false);

		sBtnConvert = pBtnConvert->GetText();

		pProgress->SetPos(0);
	}

	~ProcessArgu()
	{
		//�ͷ���Դ
		if (fpp1)
			fclose(fpp1);
		if (fpp2)
			fclose(fpp2);
		if (buf)
			free(buf);

		//�ָ��ؼ�״̬
		for (auto h : vechShouldDisable)
			h->SetEnable(true);

		FreshEdit2(*pCheckBox, *pEdit2, *pBtnDialog2);

		pBtnConvert->SetText(sBtnConvert);

		//�������״̬������
		if (success)
		{
			string caption = "������ɡ���ʱ��" + st.elapsed();
			pProgress->SetPos(100);
			MessageBox(hDlg, caption.c_str(), TEXT(""), MB_OK | MB_ICONINFORMATION);

			SetWindowText(hDlg, AppTitle);
		}
	}

	//���½�����
	void SetProgress(int per,double velocityKBperS,double remainSecond)
	{
		pProgress->SetPos(per);
		string sPercent = to_string(per) + "%";
		pBtnConvert->SetText(sPercent);

		TCHAR title[MAX_PATH];
		sprintf_s(title,MAX_PATH, "%s �ٶ�:%.0f KB/s ��ǰ:%d%% Ԥ��ʣ��:%.2f s", AppTitle,velocityKBperS,per,remainSecond);
		SetWindowText(hDlg, title);
	};
};

void DealProc(void* p)try
{
	//��p�ƽ�����ָ�����
	shared_ptr<ProcessArgu> sp((ProcessArgu*)p);
	if (fopen_s(&sp->fpp1, sp->fileName1.c_str(), "rb+"))
		throw string("��") + sp->fileName1 + string("�ļ���������ļ��Ƿ�ռ�û���û��Ȩ��");

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

	//�õ��ļ���С
	fseek(sp->fpp1, 0L, SEEK_END);
	long fileSize = ftell(sp->fpp1);
	fseek(sp->fpp1, 0L, SEEK_SET);

	//��������С �������������ļ���С����������������
	long bufSize = 0x00010000;//1MB 0x00100000

	//if (fileSize > 0x6400000)//100MB
	//	bufSize = fileSize / 50;

	sp->buf = (unsigned char*)malloc(bufSize);
	unsigned char* buf = sp->buf; //��sp�����ͷ�
	if (buf == nullptr)
		throw string("���仺�����ڴ�ʧ�ܣ���رղ���Ӧ������");

	//����λ��
	long cur = 0;
	int percent = 0;
	int prev = 0;
	unsigned char remain_buf[8];
	size_t remain_len = 0;

	//��ʼ��д
	if (fopen_s(&sp->fpp2, sp->fileName2.c_str(), "wb"))
		throw string("��") + sp->fileName2 + string("�ļ���������ļ��Ƿ�ռ�û���û��Ȩ��");


	FILE* fp1 = sp->fpp1;
	FILE* fp2 = sp->fpp2;
	long readed = 0;

	//�õ�DES��Կ
	uint64_t desKey = GetDESKeyByC(sp->desKey.c_str(), sp->desKey.length());
	if (sp->encrypt)
	{
		//����
		//�����ļ���ʽ��[RSA������С][RSA���ܺ��DES��Կ][ԭ�ļ���С][����]
		//                  4B                  ~84B            4B
		int maxBlockBytes = rsa.getMaxBlockBytes();

		unsigned char* encryptedDesKey = buf;
		memset(encryptedDesKey, 0, maxBlockBytes);
		//��Կ���Ĵ��뻺��
		WriteDESKey(desKey, encryptedDesKey);

		//д��RSA������С
		int encodedBlockSize = rsa.getEncodeBlockSize();
		fwrite(&encodedBlockSize, sizeof(encodedBlockSize), 1, fp2);

		//д��RSA����
		rsa.encodeToFile(fp2, maxBlockBytes, encryptedDesKey);

		//д��ԭ�ļ���С
		fwrite(&fileSize, sizeof(fileSize), 1, fp2);

		//�������д������
	}
	else
	{
		//����
		//����RSA������С
		int encodedBlockSize = 0;
		fread(&encodedBlockSize, sizeof(encodedBlockSize), 1, fp1);
		if (encodedBlockSize > fileSize)
			throw string("�ļ���ʽ����RSA���������ļ���С");

		//����RSA����
		unsigned char* encryptedDesKey = buf;
		fread(encryptedDesKey, encodedBlockSize, 1, fp1);

		rsa.decode(encryptedDesKey, encryptedDesKey, encodedBlockSize, encodedBlockSize);

		desKey = GetDESKeyByUC(encryptedDesKey, 8);

		//����ԭ�ļ���С
		long realSize = 0;
		readed = fread(&realSize, sizeof(realSize), 1, fp1);
		fileSize = realSize;

		//���Կ�ʼ����
	}

	while (1)
	{
		//����ļ�ʣ�ಿ��С�ڷֿ飬�򽫷ֿ���Ϊʣ���С
		bufSize = min(bufSize, fileSize - cur);

		//���ζ�ȡ�ֽ���
		readed = fread(buf, bufSize, 1, fp1) * bufSize;

		//�޶�ȡ˵���Ѷ���
		if (readed == 0)
			break;

		//����
		if (sp->encrypt)//����
			EncryptData(buf, bufSize, remain_buf, &remain_len, desKey);
		else//����
			DecryptData(buf, bufSize, desKey);

		//д��
		fwrite(buf, readed, 1, fp2);

		if (remain_len)
			fwrite(remain_buf, remain_len, 1, fp2);

		//�����Ѷ�ȡ����
		cur += readed;

		//����״̬
		percent = 100LL * cur / fileSize;
		if (sp->st.flew1sec())
		//if (percent > prev) //�ٷֱȷ����仯�ٸ���
		{
			double velocityKBperS = (cur) / 1024.0 / (sp->st.elapsedMilliseconds() / 1000.0);
			double remainSecond = (fileSize - cur) / 1024.0 / velocityKBperS;
			sp->SetProgress(percent,velocityKBperS,remainSecond);
			prev = percent;
		}
	}

	//���óɹ���ǣ�sp����λ�ؼ�
	sp->success = true;

	//�ļ������sp�ͷ�

	//sp���Զ��ͷ�
}
catch (string & err)
{
	MessageBox(hDlg, err.c_str(), TEXT("����"), MB_OK | MB_ICONERROR);
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
		BtnDecrypt.SetText("����");

		//static
		StaticDES.LinkControl(hDlg, IDC_STATIC_DES);

		StaticRSAPublic.LinkControl(hDlg, IDC_STATIC_RSA);

		StaticRSAPrivate = StaticRSAPublic;
		StaticRSAPrivate.SetText("RSA˽Կ��");

		Tab.LinkControl(hDlg, IDC_TAB);
		Tab.SetRectAsParent();

		Tab.AddTabItem("����");
		Tab.AddTabItem("����");

		Tab.TakeOverControl(0, { &StaticRSAPublic,&EditRSAPublic,&StaticDES,&EditDES,&BtnGenRSAKey,&BtnEncrypt });
		Tab.TakeOverControl(1, { &StaticRSAPrivate,&EditRSAPrivate,&BtnDecrypt });

		Tab.SetCurSel(0);

//#ifdef _DEBUG
		EditRSAPublic.SetText("C:\\Users\\jiaoshou\\Desktop\\1.pubkey");
		EditRSAPrivate.SetText("C:\\Users\\jiaoshou\\Desktop\\2.prikey");
		Edit1.SetText("C:\\Users\\jiaoshou\\Desktop\\1.zip");
		Edit2.SetText("C:\\Users\\jiaoshou\\Desktop\\2.zip");
		EditDES.SetText("����Ϊ��");
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
				if (MessageBox(hDlg, TEXT("����������Կ�ԣ�������Ҫ5����1���ӣ�ȷ��������"), TEXT(""), MB_YESNO | MB_ICONINFORMATION) == IDNO)
					break;
				RSA rsa;
				rsa.generateKeyPair();

				string publicFileName, privateFileName;
				TFileDialog fd(hDlg);
				fd.SetFilter(vecPubFilter);
				fd.SetTitle("��ѡ��Կ����λ�ã�");
				if (fd.Save(publicFileName))
				{
					rsa.savePublicKey(publicFileName.c_str());
					EditRSAPublic.SetText(publicFileName);

					fd.SetszFile("");
					fd.SetFilter(vecPriFilter);
					fd.SetTitle("��ѡ��˽Կ����λ�ã�");
					if (fd.Save(privateFileName))
					{
						rsa.savePrivateKey(privateFileName.c_str());
						EditRSAPrivate.SetText(privateFileName);

						MessageBox(hDlg, TEXT("�������."), TEXT(""), MB_OK | MB_ICONINFORMATION);
					}
				}
			}//end of try
			catch (string & err)
			{
				MessageBox(hDlg, err.c_str(), TEXT("����"), MB_OK | MB_ICONERROR);
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
				//ȡ����������ļ���
				string fileName1 = Edit1.GetText();
				string fileName2;
				if (CheckBox.GetChecked())
					fileName2 = fileName1;
				else
					fileName2 = Edit2.GetText();

				//�ļ���֤
				if (!GetFileExists(fileName1.c_str()))
					throw string("�����ļ�������");

				if (fileName2.empty())
					throw string("����ļ�Ϊ��");

				if (fileName1 == fileName2)
					throw string("����/����ļ�������ͬ");

				bool encrypt = Tab.GetCurSel() == 0;

				string desCipher;
				string keyFileName;
				if (encrypt)
				{
					//������
					desCipher = EditDES.GetText();
					int len = desCipher.length();
					if (len == 0 || len > 8)
						throw string("DES��Կ����Ϊ1��8���ַ�");

					keyFileName = EditRSAPublic.GetText();
					if (!GetFileExists(keyFileName.c_str()))
						throw string("��Կ�ļ�������");
				}
				else
				{
					keyFileName = EditRSAPrivate.GetText();
					if (!GetFileExists(keyFileName.c_str()))
						throw string("˽Կ�ļ�������");
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
				MessageBox(hDlg, err.c_str(), TEXT("����"), MB_OK | MB_ICONERROR);
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