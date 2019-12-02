#include "RSA.h"

#include <string>
#include <time.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#define EXPONENT_MAX RAND_MAX

using namespace std;

RSA::RSA():n(nullptr),e(nullptr),d(nullptr), maxBlockBytes(0)
{
	
}

RSA::~RSA()
{
	if (n)
	bignum_deinit(n);
	if (e)
	bignum_deinit(e);
	if (d)
	bignum_deinit(d);
}

void RSA::generateKeyPair(int factor_digits)
{
	bignum* p = bignum_init(), * q = bignum_init();
	bignum *phi = bignum_init();

	//n = make_shared<bignum>(bignum_init(), [](bignum* p) {bignum_deinit(p); });
	n = bignum_init();
	e = bignum_init();
	d = bignum_init();

	srand(time(NULL));

	randPrime(factor_digits, p);
	//printf("Got first prime factor, p = ");
	//bignum_print(stdout,p);
	//printf(" ... ");
	//getchar();

	randPrime(factor_digits, q);
	//printf("Got second prime factor, q = ");
	//bignum_print(q);
	//printf(" ... ");
	//getchar();

	bignum_multiply(n, p, q);
	//printf("Got modulus, n = pq = ");
	//bignum_print(n);
	//printf(" ... ");
	//getchar();

	bignum* temp1 = bignum_init(), * temp2 = bignum_init();
	bignum_subtract(temp1, p, &NUMS[1]);
	bignum_subtract(temp2, q, &NUMS[1]);
	bignum_multiply(phi, temp1, temp2); /* phi = (p - 1) * (q - 1) */
	//printf("Got totient, phi = ");
	//bignum_print(phi);
	//printf(" ... ");
	//getchar();


	randExponent(phi, EXPONENT_MAX, e);
	//printf("Chose public exponent, e = ");
	//bignum_print(e);
	//printf("\nPublic key is (");
	//bignum_print(e);
	//printf(", ");
	//bignum_print(n);
	//printf(") ... ");
	//getchar();

	bignum_inverse(e, phi, d);
	//printf("Calculated private exponent, d = ");
	//bignum_print(d);
	//printf("\nPrivate key is (");
	//bignum_print(d);
	//printf(", ");
	//bignum_print(n);
	//printf(") ... ");
	//getchar();

	bignum_deinit(p);
	bignum_deinit(q);
	bignum_deinit(phi);
	bignum_deinit(temp1);
	bignum_deinit(temp2);

	maxBlockBytes = getMaxBlockBytes();
}

int RSA::getMaxBlockBytes()
{
	if (n == nullptr)
		throw string("No public key.");

	int bytes =-1 ;//-1
	bignum* bbytes = bignum_init(), * shift = bignum_init();

	bignum_fromint(shift, 256);//1<<7 /* 7 bits per char */
	bignum_fromint(bbytes, 1);
	while (bignum_less(bbytes, n)) {
		bignum_imultiply(bbytes, shift); /* Shift by one byte, NB: we use bitmask representative so this can actually be a shift... */
		bytes++;
	}

	bignum_deinit(bbytes);
	bignum_deinit(shift);
	return bytes;
}

void RSA::checkPublicKey()
{
	if (e == nullptr || n == nullptr)
		throw string("No public key.");
}

void RSA::checkPrivateKey()
{
	if (d == nullptr || n == nullptr)
		throw string("No private key.");
}

bignum* RSA::encode(int len, char* message)
{
	checkPublicKey();
	return encodeMessage(len, maxBlockBytes, message, e, n);
}

//将 message 编码后保存至 fp
//需提前保证 len 是 maxBlockBytes 的整数倍
void RSA::encodeToFile(FILE *fp,int len, unsigned char* message)
{
	checkPublicKey();
	::encodeToFile(fp, message, len, maxBlockBytes, e, n);
}

void RSA::decode(unsigned char *result,unsigned char *message,int readSize, int encodedBlockSize)
{
	checkPrivateKey();
	::decodeFromData(result, message, readSize, encodedBlockSize, maxBlockBytes, d, n);
}

int* RSA::decode(int cryptogram_num, bignum* cryptogram)
{
	checkPrivateKey();
	return decodeMessage(cryptogram_num, maxBlockBytes, cryptogram,d,n);
}

void RSA::saveKey(const char fileName[], bignum* a, bignum* b)
{
	FILE* fp = nullptr;
	fopen_s(&fp,fileName, "w");
	if (fp == NULL)
		throw string("Fail to write the file.");

	bignum_print(fp, a);
	fwrite("\n", 1, 1, fp);
	bignum_print(fp, b);
	fclose(fp);
}

void RSA::readKey(const char fileName[], bignum *&a, bignum *&b)
{
	FILE* fp = nullptr;
	fopen_s(&fp, fileName, "r");
	if (fp == NULL)
		throw string("Fail to read the file.");

	string sa, sb;
	string* s = &sa;
	int r = 0;
	char c=0;
	while ((r = fread(&c, 1, 1, fp)) > 0)
	{
		if (c >= '0' && c <= '9')
		{
			*s += c;
			continue;
		}
		if (c == '\r') continue;
		if (c == '\n')
			if (s == &sa)
			{
				s = &sb;
				continue;
			}
			else
				break;
		throw string("Unknown char :") + c;
	}
	if (a)
		bignum_deinit(a);
	if (b)
		bignum_deinit(b);

	a=bignum_init();
	bignum_fromstring(a, sa.c_str());
	b = bignum_init();
	bignum_fromstring(b, sb.c_str());

	fclose(fp);
}

void RSA::savePublicKey(const char fileName[])
{
	checkPublicKey();
	saveKey(fileName, e, n);
}

void RSA::savePrivateKey(const char fileName[])
{
	checkPrivateKey();
	saveKey(fileName, d, n);
}

void RSA::readPublicKey(const char fileName[])
{
	readKey(fileName, e, n);
	maxBlockBytes = getMaxBlockBytes();
}

void RSA::readPrivateKey(const char fileName[])
{
	readKey(fileName, d, n);
	maxBlockBytes = getMaxBlockBytes();
}

int RSA::getEncodeBlockSize()
{
	if (n==nullptr)
		throw string("No any key.");
	return n->length*sizeof(word);
}