#include "des.h"


#ifdef _DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printuint64b(uint64_t ul)
{
	for (int i = 63; i >= 0; --i)
	{
		if ((ul >> i & 0x01) == 0)
			printf("0");
		else
			printf("1");
		if (i % 4 == 0)
			printf(" ");
	}
	printf("\n");
}

#endif

int IP[] =
{
	  58, 50, 42, 34, 26, 18, 10, 2,
	  60, 52, 44, 36, 28, 20, 12, 4,
	  62, 54, 46, 38, 30, 22, 14, 6,
	  64, 56, 48, 40, 32, 24, 16, 8,
	  57, 49, 41, 33, 25, 17,  9, 1,
	  59, 51, 43, 35, 27, 19, 11, 3,
	  61, 53, 45, 37, 29, 21, 13, 5,
	  63, 55, 47, 39, 31, 23, 15, 7
};

int PC1[] =
{
	  57, 49, 41, 33, 25, 17,  9,
	   1, 58, 50, 42, 34, 26, 18,
	  10,  2, 59, 51, 43, 35, 27,
	  19, 11,  3, 60, 52, 44, 36,
	  63, 55, 47, 39, 31, 23, 15,
	   7, 62, 54, 46, 38, 30, 22,
	  14,  6, 61, 53, 45, 37, 29,
	  21, 13,  5, 28, 20, 12,  4
};

int PC2[] =
{
	  14, 17, 11, 24,  1,  5,
	   3, 28, 15,  6, 21, 10,
	  23, 19, 12,  4, 26,  8,
	  16,  7, 27, 20, 13,  2,
	  41, 52, 31, 37, 47, 55,
	  30, 40, 51, 45, 33, 48,
	  44, 49, 39, 56, 34, 53,
	  46, 42, 50, 36, 29, 32
};

int SHIFTS[] = { 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };

int E[] =
{
	  32,  1,  2,  3,  4,  5,
	   4,  5,  6,  7,  8,  9,
	   8,  9, 10, 11, 12, 13,
	  12, 13, 14, 15, 16, 17,
	  16, 17, 18, 19, 20, 21,
	  20, 21, 22, 23, 24, 25,
	  24, 25, 26, 27, 28, 29,
	  28, 29, 30, 31, 32,  1
};

int S[8][4][16] =
{
	//S1
	14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7,
	0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8,
	4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0,
	15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13,
	//S2
	15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9,
	//S3
	10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8,
	13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1,
	13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7,
	 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12,
	 //S4
	 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15,
	13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9,
	10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4,
	 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14,
	 //S5
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9,
	14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6,
	 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14,
	11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3,
	//S6
	12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11,
	10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8,
	 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6,
	 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13,
	 //S7
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1,
	13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6,
	 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2,
	 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12,
	 //S8
	13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7,
	 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2,
	 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8,
	 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

int P[] =
{
	  16,  7, 20, 21,
	  29, 12, 28, 17,
	   1, 15, 23, 26,
	   5, 18, 31, 10,
	   2,  8, 24, 14,
	  32, 27,  3,  9,
	  19, 13, 30,  6,
	  22, 11,  4, 25
};

int IP1[] =
{
	  40, 8, 48, 16, 56, 24, 64, 32,
	  39, 7, 47, 15, 55, 23, 63, 31,
	  38, 6, 46, 14, 54, 22, 62, 30,
	  37, 5, 45, 13, 53, 21, 61, 29,
	  36, 4, 44, 12, 52, 20, 60, 28,
	  35, 3, 43, 11, 51, 19, 59, 27,
	  34, 2, 42, 10, 50, 18, 58, 26,
	  33, 1, 41,  9, 49, 17, 57, 25
};

uint64_t uint64convert(uint64_t ul, int C[], int C_len)
{
	uint64_t ret = 0;
	for (int i = 0; i < C_len; ++i)
	{
		//t最后一位存查表结果 0或1
		uint64_t t = ul >> (64 - C[i]) & 0x01;

		//将该位移到位
		t <<= (63 - (uint64_t)i);
		ret |= t;
	}
	return ret;
}

// C  D  0
//28 28  8 = 64
uint64_t CDshift(uint64_t CD, int turn)
{
	//CD 每28位拆分
	CD >>= 8;
	uint32_t D = CD & 0x0FFFFFFF;
	CD >>= 28;
	uint32_t C = (uint32_t)CD;
	int s = SHIFTS[turn - 1];

	//循环移位：移s位，则将s位保存至res
	uint32_t Cres = C >> (28 - s);
	uint32_t Dres = D >> (28 - s);

	//合并res以完成循环移位
	C = C << s | Cres;
	D = (D << s | Dres) & 0x0FFFFFFF;

	//组合C D
	uint64_t ret = 0;
	ret = (uint64_t)C << (28 + 8);
	ret |= (uint64_t)D << 8;
	return ret;
}

uint64_t SBox(uint64_t B)
{
	uint64_t ret = 0;
	for (int k = 0; k < 8; ++k)
	{
		//uc: 00 uc
		unsigned char uc = B >> (16 + (7 - k) * 6) & 0x3F;

		//uc首尾各1位组成 0-3 的行号
		unsigned char i = (uc >> 4 & 0x02) | (uc & 0x01);

		//uc去头尾4位组成列号 0-15
		unsigned char j = uc >> 1 & 0x0F;

		//查SBOX
		uint64_t s = S[k][i][j];

		//拼出S
		ret |= s << (60 - k * 4);
	}
	return ret;
}

//加解密为同一函数，仅使用K顺序不同
uint64_t Do(uint64_t plain, uint64_t key, int encrypt)
{
	//PC-1
	uint64_t CD = uint64convert(key, PC1, 56);//K56bits = C0D0

	//IP
	uint64_t IPtext = uint64convert(plain, IP, 64);
	uint64_t L = IPtext & 0xFFFFFFFF00000000;
	uint64_t R = IPtext << 32;

	//密钥组
	uint64_t K[16];
	for (int i = 1; i <= 16; ++i)
	{
		//SHIFT
		CD = CDshift(CD, i);//CD1 -> CD16

		//PC-2
		K[i - 1] = uint64convert(CD, PC2, 48);//K1 -> K16
	}

	for (int i = 1; i <= 16; ++i)
	{
		//Ln
		uint64_t Ln = R;

		//E
		uint64_t ER = uint64convert(R, E, 48);

		//B
		uint64_t B = 0;

		if (encrypt)
			//加密：使用K1 ... K16
			B = ER ^ K[i - 1];
		else
			//解密：使用K16 ... K1
			B = ER ^ K[16 - i];

		//SBox
		uint64_t S = SBox(B);

		//P
		uint64_t f = uint64convert(S, P, 32);

		//Rn=Ln-1 ^ f(ERn ^ K)
		uint64_t Rn = L ^ f;

		L = Ln;
		R = Rn;
	}

	uint64_t RL = R | (L >> 32);

	uint64_t C = uint64convert(RL, IP1, 64);

	return C;
}

//加密
uint64_t Encrypt(uint64_t plain, uint64_t key)
{
	return Do(plain, key, 1);
}

//解密
uint64_t Decrypt(uint64_t plain, uint64_t key)
{
	return Do(plain, key, 0);
}

union charTo64
{
	unsigned char s[8];
	uint64_t u;
};

uint64_t GetDESKeyByUC(const unsigned char* s, int len)
{
	union charTo64 ct;
	memset(ct.s, 0, 8);
	memcpy_s(ct.s, 8, s, len);
	return ct.u;
}

uint64_t GetDESKeyByC(const char* s, int len)
{
	union charTo64 ct;
	memset(ct.s, 0, 8);
	memcpy_s(ct.s, 8, s, len);
	return ct.u;
}

void WriteDESKey(uint64_t key, unsigned char* s)
{
	union charTo64 ct;
	ct.u = key;
	memcpy_s(s, 8, ct.s, 8);
}

//unsigned char remain_buf[8];
void EncryptData(unsigned char* plain, size_t len, unsigned char* remain_buf, size_t* remain_len, uint64_t key)
{
	size_t block = len / 8;
	*remain_len = len % 8;

	uint64_t* u = (uint64_t*)plain;
	while (block--)
	{
		*u = Encrypt(*u, key);
		u++;
	}

	union charTo64 ct;

	if (*remain_len > 0)
	{
		memset(ct.s, 0, 8);
		memcpy_s(ct.s, 8, plain + block * 8, *remain_len);
		ct.u = Encrypt(ct.u, key);
		memcpy_s(remain_buf, 8, ct.s, 8);
	}
}

//unsigned char remain_buf[8];
void DecryptData(unsigned char* plain, size_t len,  uint64_t key)
{
	size_t block = len / 8;

	uint64_t* u = (uint64_t*)plain;
	while (block--)
	{
		*u = Decrypt(*u, key);
		u++;
	}
}