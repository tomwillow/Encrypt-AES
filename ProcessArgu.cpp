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
	//��p�ƽ�����ָ�����
	shared_ptr<ProcessArgu> sp((ProcessArgu*)p);

	if (fopen_s(&sp->fpp1, sp->fileName1.c_str(), "rb"))
		throw MyException(sp->hDlg,string("��") + sp->fileName1 + string("�ļ���������ļ��Ƿ�ռ�û���û��Ȩ��"));

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

	//�õ��ļ���С
	fseek(sp->fpp1, 0L, SEEK_END);
	long fileSize = ftell(sp->fpp1);
	fseek(sp->fpp1, 0L, SEEK_SET);

	//��������С
	long defBufSize = 0x00010000;//1MB 0x00100000

	//��ʼ��д
	if (fopen_s(&sp->fpp2, sp->fileName2.c_str(), "wb"))
		throw MyException(sp->hDlg,string("��") + sp->fileName2 + string("�ļ���������ļ��Ƿ�ռ�û���û��Ȩ��"));


	FILE* fp1 = sp->fpp1;
	FILE* fp2 = sp->fpp2;

	//д��ƫ����
	long offset = 0;

	//�õ�DES��Կ
	uint64_t desKey = GetDESKeyByC(sp->desKey.c_str(), sp->desKey.length());

	if (sp->encrypt)
	{
		//����
		//�����ļ���ʽ��[RSA������С][RSA���ܺ��DES��Կ][ԭ�ļ���С][����]
		//                  4B                  ~84B            4B
		int maxBlockBytes = rsa.getMaxBlockBytes();
		int encodedBlockSize = rsa.getEncodeBlockSize();
		int sz = max(maxBlockBytes, encodedBlockSize);

		//����ռ�
		shared_ptr<unsigned char> encryptedDesKey(new unsigned char[sz]);

		//�����г�ʼ�����൱��ǰ8B��DES��Կ������������
		//����ʱ����������ݣ�ֻȡǰ8B
		//memset(encryptedDesKey.get(), 0, sz);

		//��Կ���Ĵ��뻺�棬ֻд��ǰ8B
		WriteDESKey(desKey, encryptedDesKey.get());

		//д��RSA������С
		fwrite(&encodedBlockSize, sizeof(encodedBlockSize), 1, fp2);

		//д��RSA����
		rsa.encodeToFile(fp2, maxBlockBytes, encryptedDesKey.get());

		//д��ԭ�ļ���С
		fwrite(&fileSize, sizeof(fileSize), 1, fp2);

		offset = 4 + encodedBlockSize + 4;
		//�������д������
	}
	else
	{
		//����
		//����RSA������С
		int encodedBlockSize = 0;
		fread(&encodedBlockSize, sizeof(encodedBlockSize), 1, fp1);

		if (encodedBlockSize > fileSize)
			throw MyException(sp->hDlg,string("�ļ���ʽ����RSA���������ļ���С"));

		int sz = max(encodedBlockSize, rsa.getMaxBlockBytes());

		//����ռ�
		shared_ptr<unsigned char> encryptedDesKey(new unsigned char[sz]);

		//����RSA����
		fread(encryptedDesKey.get(), encodedBlockSize, 1, fp1);

		//RSA����
		rsa.decode(encryptedDesKey.get(), encryptedDesKey.get(), encodedBlockSize, encodedBlockSize);

		//ȡ��DES��Կ
		desKey = GetDESKeyByUC(encryptedDesKey.get(), 8);

		//����ԭ�ļ���С
		fread(&fileSize, sizeof(fileSize), 1, fp1);
		//֮���д���Ⱦ�ΪfileSize

		offset = 4 + encodedBlockSize + 4;
		//���Կ�ʼ����
	}
	//����fread�Ķ�ȡƫ�ƣ�֮���ȡ����ΪfileSize
	//֮��д������ΪfileSize
	sp->dealBytes = fileSize;

	mutex dealMutex, writeMutex;

	//��ȡ�ṹ
	struct TData
	{
		long bufSize;
		unsigned char* buf;

		long shouldWriteTo;//�ò���Ӧд���λ��
	};

	queue<TData> qReaded;

	struct TWriteData
	{
		long pos;//д��λ��
		long writeBytes;//д������
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
			//����ļ�ʣ�ಿ��С�ڷֿ飬�򽫷ֿ���Ϊʣ���С
			bufSize = min(bufSize, fileSize - cur);

			unsigned char* buf = new unsigned char[bufSize];

			//���ζ�ȡ�ֽ���
			long readed = fread(buf, bufSize, 1, fp) * bufSize;

			//��ȡ��Ϊ0˵����ȡ���
			if (readed == 0)
				break;

			TData data;
			data.buf = buf;
			data.bufSize = readed;
			data.shouldWriteTo = cur + (encrypt ? offset : 0);

			cur += readed;

			//�������������账�����
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

			//���겢�Ѵ����꣬���˳��߳�
			if (readFinish == true && qReadedEmpty)
			{
				dealMutex.unlock();
				return;
			}

			if (!qReadedEmpty)
			{
				//ȡ���������
				readed = qReaded.front();
				qReaded.pop();
			}
			dealMutex.unlock();

			if (qReadedEmpty)
				this_thread::sleep_for(100ms);//δ���꣬��û�д������ ������
			else
			{
				//��д���
				TWriteData writeData;
				writeData.writeBytes = readed.bufSize;
				writeData.buf = readed.buf;
				writeData.pos = readed.shouldWriteTo;
				writeData.remain_len = 0;

				//����
				if (encrypt)//����
					EncryptData(readed.buf, readed.bufSize, writeData.remain_buf, &(writeData.remain_len), desKey);
				else//����
					DecryptData(readed.buf, readed.bufSize, desKey);

				//�����д�����
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

				//д��
				fwrite(writeData.buf, writeData.writeBytes, 1, fp);
				delete[] writeData.buf;

				if (writeData.remain_len)
					fwrite(writeData.remain_buf, writeData.remain_len, 1, fp);

				wrote += writeData.writeBytes + writeData.remain_len;
			}
			else
			{
				//δ���д�룬����û�д�д��� ������
				this_thread::sleep_for(100ms);
			}

			//����״̬
			long percent = 100LL * wrote / fileSize;
			if (sp->st.flew1sec())//�����ϴθ����ѳ���1��
			{
				double velocityKBperS = (wrote) / 1024.0 / (sp->st.elapsedMilliseconds() / 1000.0);
				double remainSecond = (fileSize - wrote) / 1024.0 / velocityKBperS;
				sp->SetProgress(percent, velocityKBperS, remainSecond);
			}
		}
	};

	thread t_read(Reader, fp1, fileSize, defBufSize, sp->encrypt);
	t_read.join();

	//ȡ��CPU�߼�������
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

	//���óɹ���ǣ�sp����λ�ؼ�
	sp->success = true;

	//�ļ������sp�ͷ�

	//sp���Զ��ͷ�
}
catch (MyException &e)
{
	MessageBox(e.m_hDlg, e.m_info.c_str(), TEXT("����"), MB_OK | MB_ICONERROR);
}