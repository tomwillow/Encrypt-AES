#include "ProcessArgu.h"

#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <exception>

#include "RSA/RSA.h"
extern "C"
{
#include "DES/des.h"
}

using namespace std;

class MyException :public std::exception
{
public:
	HWND m_hDlg;
	string m_info;
public:
	MyException(HWND hDlg, string info) :m_hDlg(hDlg), m_info(info) {}
};

void DealProc(void* p)try
{
	//将p移交智能指针管理
	shared_ptr<ProcessArgu> sp((ProcessArgu*)p);

	if (fopen_s(&sp->fpp1, sp->fileName1.c_str(), "rb"))
		throw MyException(sp->hDlg,string("打开") + sp->fileName1 + string("文件出错，检查文件是否被占用或者没有权限"));

	sp->SetProgress(0, 0.0, -1);

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

	//缓冲区大小
	long defBufSize = 0x00010000;//1MB 0x00100000

	//开始读写
	if (fopen_s(&sp->fpp2, sp->fileName2.c_str(), "wb"))
		throw MyException(sp->hDlg,string("打开") + sp->fileName2 + string("文件出错，检查文件是否被占用或者没有权限"));


	FILE* fp1 = sp->fpp1;
	FILE* fp2 = sp->fpp2;

	//写入偏移量
	long offset = 0;

	//得到DES密钥
	uint64_t desKey = GetDESKeyByC(sp->desKey.c_str(), sp->desKey.length());

	if (sp->encrypt)
	{
		//加密
		//密文文件格式：[RSA编码块大小][RSA加密后的DES密钥][原文件大小][密文]
		//                  4B                  ~84B            4B
		int maxBlockBytes = rsa.getMaxBlockBytes();
		int encodedBlockSize = rsa.getEncodeBlockSize();
		int sz = max(maxBlockBytes, encodedBlockSize);

		//分配空间
		shared_ptr<unsigned char> encryptedDesKey(new unsigned char[sz]);

		//不进行初始化，相当于前8B存DES密钥，后面随机填充
		//解密时获得整个数据，只取前8B
		//memset(encryptedDesKey.get(), 0, sz);

		//密钥明文存入缓存，只写入前8B
		WriteDESKey(desKey, encryptedDesKey.get());

		//写入RSA编码块大小
		fwrite(&encodedBlockSize, sizeof(encodedBlockSize), 1, fp2);

		//写入RSA密文
		rsa.encodeToFile(fp2, maxBlockBytes, encryptedDesKey.get());

		//写入原文件大小
		fwrite(&fileSize, sizeof(fileSize), 1, fp2);

		offset = 4 + encodedBlockSize + 4;
		//下面可以写入密文
	}
	else
	{
		//解密
		//读出RSA编码块大小
		int encodedBlockSize = 0;
		fread(&encodedBlockSize, sizeof(encodedBlockSize), 1, fp1);

		if (encodedBlockSize > fileSize)
			throw MyException(sp->hDlg,string("文件格式错误：RSA编码块大于文件大小"));

		int sz = max(encodedBlockSize, rsa.getMaxBlockBytes());

		//分配空间
		shared_ptr<unsigned char> encryptedDesKey(new unsigned char[sz]);

		//读出RSA密文
		fread(encryptedDesKey.get(), encodedBlockSize, 1, fp1);

		//RSA解密
		rsa.decode(encryptedDesKey.get(), encryptedDesKey.get(), encodedBlockSize, encodedBlockSize);

		//取得DES密钥
		desKey = GetDESKeyByUC(encryptedDesKey.get(), 8);

		//读出原文件大小
		fread(&fileSize, sizeof(fileSize), 1, fp1);
		//之后读写长度均为fileSize

		offset = 4 + encodedBlockSize + 4;
		//可以开始解密
	}
	//由于fread的读取偏移，之后读取数均为fileSize
	//之后写入总量为fileSize
	sp->dealBytes = fileSize;

	mutex dealMutex, writeMutex;

	//读取结构
	struct TData
	{
		long bufSize;
		unsigned char* buf;

		long shouldWriteTo;//该部分应写入的位置
	};

	queue<TData> qReaded;

	struct TWriteData
	{
		long pos;//写入位置
		long writeBytes;//写入数量
		unsigned char* buf;

		size_t remain_len;
		unsigned char remain_buf[8];
	};

	queue<TWriteData> qWrite;

	bool readFinish = false;

	auto Reader = [&qReaded, &dealMutex, &offset, &readFinish](FILE* fp, long fileSize, long bufSize, bool encrypt)
	{
		long cur = 0;
		while (1)
		{
			//如果文件剩余部分小于分块，则将分块设为剩余大小
			bufSize = min(bufSize, fileSize - cur);

			unsigned char* buf = new unsigned char[bufSize];

			//本次读取字节数
			long readed = fread(buf, bufSize, 1, fp) * bufSize;

			//读取数为0说明读取完成
			if (readed == 0)
				break;

			TData data;
			data.buf = buf;
			data.bufSize = readed;
			data.shouldWriteTo = cur + (encrypt ? offset : 0);

			cur += readed;

			//上锁，并加入需处理队列
			dealMutex.lock();
			qReaded.push(data);
			dealMutex.unlock();
		}
		readFinish = true;
	};

	auto Dealer = [&dealMutex, &qReaded, &desKey, &writeMutex, &qWrite, &readFinish](bool encrypt)
	{
		while (1)
		{
			bool qReadedEmpty = false;
			TData readed;
			dealMutex.lock();
			qReadedEmpty = qReaded.empty();

			//读完并已处理完，则退出线程
			if (readFinish == true && qReadedEmpty)
			{
				dealMutex.unlock();
				return;
			}

			if (!qReadedEmpty)
			{
				//取出待处理块
				readed = qReaded.front();
				qReaded.pop();
			}
			dealMutex.unlock();

			if (qReadedEmpty)
				this_thread::sleep_for(100ms);//未读完，但没有待处理块 则阻塞
			else
			{
				//待写入块
				TWriteData writeData;
				writeData.writeBytes = readed.bufSize;
				writeData.buf = readed.buf;
				writeData.pos = readed.shouldWriteTo;
				writeData.remain_len = 0;

				//处理
				if (encrypt)//加密
					EncryptData(readed.buf, readed.bufSize, writeData.remain_buf, &(writeData.remain_len), desKey);
				else//解密
					DecryptData(readed.buf, readed.bufSize, desKey);

				//加入待写入队列
				writeMutex.lock();
				qWrite.push(writeData);
				writeMutex.unlock();
			}
		}
	};

	auto Writer = [&writeMutex, &qWrite, &sp](FILE* fp, long fileSize)
	{
		long wrote = 0;
		while (wrote < fileSize)
		{
			bool couldWrite = false;
			TWriteData writeData;

			writeMutex.lock();
			if (!qWrite.empty())
			{
				writeData = qWrite.front();
				qWrite.pop();
				couldWrite = true;
			}
			writeMutex.unlock();

			if (couldWrite)
			{
				fseek(fp, writeData.pos, SEEK_SET);

				//写入
				fwrite(writeData.buf, writeData.writeBytes, 1, fp);
				delete[] writeData.buf;

				if (writeData.remain_len)
					fwrite(writeData.remain_buf, writeData.remain_len, 1, fp);

				wrote += writeData.writeBytes + writeData.remain_len;
			}
			else
			{
				//未完成写入，但又没有待写入块 则阻塞
				this_thread::sleep_for(100ms);
			}

			//更新状态
			long percent = 100LL * wrote / fileSize;
			if (sp->st.flew1sec())//距离上次更新已超过1秒
			{
				double velocityKBperS = (wrote) / 1024.0 / (sp->st.elapsedMilliseconds() / 1000.0);
				double remainSecond = (fileSize - wrote) / 1024.0 / velocityKBperS;
				sp->SetProgress(percent, velocityKBperS, remainSecond);
			}
		}
	};

	thread t_read(Reader, fp1, fileSize, defBufSize, sp->encrypt);
	t_read.join();

	//取得CPU逻辑核数量
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int i = sysInfo.dwNumberOfProcessors;
	while (i--)
	{
		thread t_deal(Dealer, sp->encrypt);
		t_deal.detach();
	}

	thread t_write(Writer, fp2, fileSize);
	t_write.join();

	//设置成功标记，sp将复位控件
	sp->success = true;

	//文件句柄由sp释放

	//sp将自动释放
}
catch (MyException &e)
{
	MessageBox(e.m_hDlg, e.m_info.c_str(), TEXT("错误"), MB_OK | MB_ICONERROR);
}