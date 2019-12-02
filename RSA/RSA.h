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
	//max�� -> encoded��
	int getMaxBlockBytes();

	int getEncodeBlockSize();

	bignum* encode(int len, char* message);

	//�� message ����󱣴��� fp
	//����ǰ��֤ len �� maxBlockBytes ��������
	//ʵ��д���СΪ getEncodeBlockSize
	void encodeToFile(FILE *fp,int len,unsigned char* message);

	int* decode(int len, bignum* cryptogram);

	//�� message ���н���
	//readSize ����Ϊ encodedBlockSize ��������
	//���д�� result��result ��������>= maxBlockBytes * iBlock����
	void decode(unsigned char* result, unsigned char* message, int readSize, int encodedBlockSize);

	void savePublicKey(const char fileName[]);
	void savePrivateKey(const char fileName[]);

	void readPublicKey(const char fileName[]);
	void readPrivateKey(const char fileName[]);
};

