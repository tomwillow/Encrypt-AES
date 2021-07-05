#include "ProcessArgu.h"

#include <memory>
#include <exception>

#include "MyAES.h"
#include "MyAESQuick.h"

using namespace std;

//自定义异常类
//目的为传递hDlg，因此异常捕捉时就不需要设置全局hDlg变量了
class MyException :public std::exception
{
public:
	HWND m_hDlg;
	tstring m_info;
public:
	MyException(HWND hDlg, tstring info) :m_hDlg(hDlg), m_info(info) {}
};


//更新进度条
void ProcessArgu::SetProgress(int per, double velocityKBperS, double remainSecond)
{
	//设置按钮文字为百分比
	pProgress->SetPos(per);
	std::tstring sPercent = std::tto_string(per) + TEXT("%");
	pBtnConvert->SetText(sPercent);

	//设置窗口标题显示速度
	TCHAR title[MAX_PATH];
	_stprintf_s(title, MAX_PATH, TEXT("%s 速度:%.0f KB/s 当前:%d%% 预计剩余:%.2f s"), AppTitle, velocityKBperS, per, remainSecond);
	SetWindowText(hDlg, title);
};

void DealProc(void* p)try
{
	//将p移交智能指针管理
	unique_ptr<ProcessArgu> sp((ProcessArgu*)p);

	//创建文件句柄并移交智能指针管理
	FILE* fp1 = nullptr;
	FILE* fp2 = nullptr;
	unique_ptr<FILE*, void(*)(FILE**)> fp1_shared(&fp1, [](FILE** fpp) {fclose(*fpp); });
	unique_ptr<FILE*, void(*)(FILE**)> fp2_shared(&fp2, [](FILE** fpp) {fclose(*fpp); });

	//打开输入文件
	if (_tfopen_s(&fp1, sp->fileName1.c_str(), TEXT("rb")))
		throw MyException(sp->hDlg,tstring(TEXT("打开")) + sp->fileName1 + tstring(TEXT("文件出错，检查文件是否被占用或者没有权限")));

	//设置初始进度
	sp->SetProgress(0, 0.0, -1);

	//得到文件大小
	fseek(fp1, 0L, SEEK_END);
	long long fileSize=_ftelli64(fp1);
	fseek(fp1, 0L, SEEK_SET);

	//缓冲区大小,
	//***必须为16的倍数，否则每次读写都会产生碎片，文件大小校验将不通过
	long defBufSize = 0x00010000;//1MB 0x00100000

	//由于1位的分组大小运算缓慢，为了进度能够刷新，将缓冲区调小
	if (sp->mode == MyAES::CFB1 || sp->mode == MyAES::OFB1)
		defBufSize = 0x1000;

	//分配缓冲区并移交智能指针管理
	long bufSize = defBufSize;
	unsigned char* buf = new unsigned char[bufSize];
	unique_ptr<unsigned char>buf_shared(buf);
	if (buf == nullptr)
		throw MyException(sp->hDlg, tstring(TEXT("分配缓冲区内存失败。请关闭部分程序后再试。")));

	//打开输出文件
	if (_tfopen_s(&fp2, sp->fileName2.c_str(), TEXT("wb")))
		throw MyException(sp->hDlg,tstring(TEXT("打开")) + sp->fileName2 + tstring(TEXT("文件出错，检查文件是否被占用或者没有权限")));

	//分配key空间
	size_t key_len = sp->key.length();
	unsigned char* key = new unsigned char[key_len];
	unique_ptr<unsigned char> key_shared(key);
	memcpy(key, sp->key.c_str(), key_len);

	//分配iv空间
	size_t iv_len = sp->iv.length();
	unsigned char* iv = new unsigned char[iv_len];
	unique_ptr<unsigned char> iv_shared(iv);
	memcpy(iv, sp->iv.c_str(), iv_len);

	//实例化加密类，并移交智能指针管理
	MyAES* aes = nullptr;
	if (sp->quick)//选中查表加速，则使用快速类
		aes = new MyAESQuick(key, key_len, sp->bits, (MyAES::AESMode) sp->mode, iv, iv_len);
	else
		aes =new MyAES(key, key_len, sp->bits, (MyAES::AESMode) sp->mode, iv, iv_len);
	unique_ptr<MyAES> aes_shared(aes);

	//padding变量只用于解密时判断是否有填充
	long padding = 0;
	if (sp->encrypt)
	{
		//加密头
		//格式：4B:[0-3]    8B:[4-11]     4B:[12-15]
		//内容：  AES        fileSize      padding
		unsigned char header[16];
		memset(header, 0, 16);

		//存入标志符“AES”
		memcpy(header, "AES", 4);

		//原文件大小
		long long* pFileSize = (long long*)&header[4];
		*pFileSize = fileSize;

		//padding大小
		long* pPadding = (long*)&header[4+sizeof(fileSize)];
		*pPadding = 0;
		if (sp->mode == MyAES::ECB || sp->mode == MyAES::CBC || sp->mode == MyAES::CTR)
			if (fileSize%16)
				*pPadding = 16-fileSize % 16;

		//加密文件头
		aes->Encrypt(header,16);

		//写入文件头
		fwrite(header, 16, 1, fp2);
	}
	else
	{
		//读出文件头并解密
		unsigned char header[16];
		size_t read=fread(header, 16, 1, fp1);
		aes->Decrypt(header, 16);

		//取出标志符“AES”
		unsigned char symbol[4];
		memcpy(symbol, header, 4);

		//前三个字节不是AES，说明bits, mode, key, iv中有不正确之处
		//或者根本就不是加密后的文件
		if (read==0 || memcmp(symbol, "AES", 3) != 0)
			throw MyException(sp->hDlg, TEXT("解密失败。检查位宽、模式、密钥或IV是否正确。"));

		//读取原文件大小
		long long* pOriginFileSize = (long long*)&header[4];
		long long originFileSize = *pOriginFileSize;
		long long encrypedFileSize = 16 + originFileSize;

		//读取padding大小
		long* pPadding = (long*)&header[4+sizeof(fileSize)];
		padding = *pPadding;

		//修正文件大小
		if (padding)
			encrypedFileSize += padding;

		//进一步进行文件大小校验
		if (fileSize!= encrypedFileSize)
			throw MyException(sp->hDlg, TEXT("输入文件大小不正确，内容可能已经被破坏"));

		//将文件大小设置为剥去文件头的大小，
		//如果有padding的话，是原文件加上padding的大小
		fileSize = encrypedFileSize - 16;
	}

	long long cur = 0;
	while (1)
	{
		//如果文件剩余部分小于分块，则将分块设为剩余大小
		bufSize = (long)min(bufSize, fileSize - cur);

		//本次读取字节数
		long readed = fread(buf, bufSize, 1, fp1) * bufSize;

		//读取数为0说明读取完成
		if (readed == 0)
			break;

		if (sp->encrypt)
		{
			//加密，并返回本轮加密在buf上更改的字节数
			long deal = aes->Encrypt(buf, readed);

			//写入整块数据
			fwrite(buf, deal, 1, fp2);

			//更改字节和送入字节不一致，说明有碎片
			if (deal != readed)
			{
				//将padding后加密的碎片存入
				fwrite(aes->remain_buf, 16, 1, fp2);
			}
		}
		else
		{
			//解密
			aes->Decrypt(buf, readed);

			if (padding && fileSize - cur == bufSize)//存在padding并且为最后一块
			{
				//将待写入数量减去padding
				readed -= padding;
			}
			fwrite(buf, readed, 1, fp2);
		}

		//cur为当前已处理数量
		cur += readed;

		//更新状态
		long percent = (long)(100LL * cur / fileSize);
		if (sp->st.flew1sec())//距离上次更新已超过1秒
		{
			double velocityKBperS = (cur) / 1024.0 / (sp->st.elapsedMilliseconds() / 1000.0);
			double remainSecond = (fileSize - cur) / 1024.0 / velocityKBperS;
			sp->SetProgress(percent, velocityKBperS, remainSecond);
		}
	}

	//设置总处理数量，用于在最终弹框时显示总速度
	sp->dealBytes = fileSize+16;

	//设置成功标记，sp将复位控件
	sp->success = true;

	//文件句柄，各buf均由各自智能指针释放

	//sp将自动释放
}
catch (MyException &e)
{
	MessageBox(e.m_hDlg, e.m_info.c_str(), TEXT("错误"), MB_OK | MB_ICONERROR);
}