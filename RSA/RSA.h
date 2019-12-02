#pragma once
extern "C"
{
#include "multiple.h"
}
#include <memory>

class RSA
{
private:
	//std::shared_ptr<bignum> n;
	bignum *n;
	bignum* e;
	bignum* d;

	int maxBlockBytes;
	void checkPublicKey();
	void checkPrivateKey();
	void saveKey(const char fileName[], bignum* a, bignum* b);
	void readKey(const char fileName[], bignum*& a, bignum*& b);
public:
	RSA();
	~RSA();

	void generateKeyPair(int factor_digits=100);

	/* Compute maximum number of bytes that can be encoded in one encryption */
	//max块 -> encoded块
	int getMaxBlockBytes();

	int getEncodeBlockSize();

	bignum* encode(int len, char* message);

	//将 message 编码后保存至 fp
	//需提前保证 len 是 maxBlockBytes 的整数倍
	//实际写入大小为 getEncodeBlockSize
	void encodeToFile(FILE *fp,int len,unsigned char* message);

	int* decode(int len, bignum* cryptogram);

	//将 message 进行解密
	//readSize 必须为 encodedBlockSize 的整数倍
	//结果写入 result，result 容量必须>= maxBlockBytes * iBlock块数
	void decode(unsigned char* result, unsigned char* message, int readSize, int encodedBlockSize);

	void savePublicKey(const char fileName[]);
	void savePrivateKey(const char fileName[]);

	void readPublicKey(const char fileName[]);
	void readPrivateKey(const char fileName[]);
};

