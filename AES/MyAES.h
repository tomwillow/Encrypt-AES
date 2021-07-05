#pragma once
#include <stdint.h>
#include <iostream>
#include <string>

#include <stdlib.h>
#include <string.h>

class MyAES
{
public:
	enum AESMode { ECB, CBC,CTR,CFB1,CFB8,OFB1,OFB8 };
	uint8_t remain_buf[16];
protected:
	//Nb = 4
	const int Nb;

	//when [128 192 256] bits, round = [10 12 14]
	int round;

	//when [128 192 256] bits, iv_bytes = Nb * 4 = [16]
	const int iv_bytes;

	//256 bits: size of key = 32
	uint8_t key[32];

	//256 bits: 32b of expandedKey = Nb * (14 + 1) = 60
	uint32_t expandedKey[60];

private:
	uint8_t iv[16];

	const AESMode mode;

	//while [128 192 256] bits, Nk = [4 6 8]
	const int Nk;

	//blockSize = 16
	const int blockSize;
public:
	MyAES(const uint8_t *key, int key_len, int bits = 128,AESMode mode=AESMode::ECB,const uint8_t *iv=nullptr,int iv_len=0) :
		Nb(4),iv_bytes(16), mode(mode), Nk(bits / 32), blockSize(Nb*4)
	{
		switch (bits)
		{
		case 128:
			round = 10; break;
		case 192:
			round = 12; break;
		case 256:
			round = 14; break;
		default:
			throw std::string("不支持的位宽:")+std::to_string(bits);
		}

		if (key_len > Nk*4)
			throw std::string("密钥最大长度为") + std::to_string(Nk*4) + "Bytes";

		if (iv_len > Nk*4)
			throw std::string("iv最大长度为") + std::to_string(iv_bytes) + "Bytes";

		//key和iv小于Nk*4，将填充0
		memset(this->key, 0, Nk*4);
		memcpy_s(this->key, key_len, key, key_len);

		memset(this->iv, 0, iv_bytes);
		if (iv!=nullptr)
			memcpy_s(this->iv, iv_len, iv, iv_len);
	}

	~MyAES()
	{
	}

	size_t GetBlockSize()
	{
		return blockSize;
	}

	//如果len不是block整数倍，则将
	//多余部分加密后存入remain_buf
	//返回加密数量
	size_t Encrypt(uint8_t plain[], size_t len);

	//如果len不是block整数倍，在ECB和CBC模式下将只处理整块数据
	void Decrypt(uint8_t cipher[], size_t len);
private:
	static const uint8_t sBox[];
	static const uint8_t invsBox[];

	static uint32_t RotByte(uint32_t u);
	static uint32_t SubByte(uint32_t u);

	virtual uint8_t GFMul(int n,uint32_t u) const;
	static uint8_t GFMul2(uint32_t u);

	void ByteSub(uint8_t state[]) const;
	void InvByteSub(uint8_t state[]) const;

	void ShiftRow(uint8_t state[]) const;
	void InvShiftRow(uint8_t state[]) const;

	void MixColumn(uint8_t state[]) const;
	void InvMixColumn(uint8_t state[]) const;
protected:

	virtual void EncryptBlock(uint8_t state[]);
	virtual void DecryptBlock(uint8_t state[]);

	void KeyExpansion(const uint8_t key[], uint32_t W[]) const;

	void AddRoundKey(uint8_t state[], const uint32_t expandedKey[]) const;

	void Round(uint8_t state[], const uint32_t roundKey[]) const;
	void InvRound(uint8_t state[], const uint32_t roundKey[]) const;

	void FinalRound(uint8_t state[], const uint32_t roundKey[]) const;
	void InvFinalRound(uint8_t state[], const uint32_t roundKey[]) const;
};

