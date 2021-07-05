#include "MyAES.h"

#include <random>
#include <bitset>

using namespace std;

const uint8_t MyAES::sBox[] =
{ /*  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */
	0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76, /*0*/
	0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0, /*1*/
	0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15, /*2*/
	0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75, /*3*/
	0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84, /*4*/
	0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf, /*5*/
	0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8, /*6*/
	0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2, /*7*/
	0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73, /*8*/
	0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb, /*9*/
	0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79, /*a*/
	0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08, /*b*/
	0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a, /*c*/
	0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e, /*d*/
	0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf, /*e*/
	0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16  /*f*/
};

const uint8_t MyAES::invsBox[] =
{ /*  0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f  */
	0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb, /*0*/
	0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb, /*1*/
	0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e, /*2*/
	0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25, /*3*/
	0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92, /*4*/
	0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84, /*5*/
	0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06, /*6*/
	0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b, /*7*/
	0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73, /*8*/
	0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e, /*9*/
	0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b, /*a*/
	0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4, /*b*/
	0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f, /*c*/
	0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef, /*d*/
	0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61, /*e*/
	0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d  /*f*/
};


void MyAES::AddRoundKey(uint8_t state[], const uint32_t expandedKey[]) const
{
	uint32_t* pstate = (uint32_t*)state;
	for (int i = 0; i < Nb; ++i)
		pstate[i] ^= expandedKey[i];
}

void MyAES::ByteSub(uint8_t state[]) const
{
	for (int i = 0; i < blockSize; ++i)
	{
		state[i] = sBox[(uint32_t)state[i]];
	}
}

void MyAES::InvByteSub(uint8_t state[]) const
{
	for (int i = 0; i < blockSize; ++i)
	{
		state[i] = invsBox[(uint32_t)state[i]];
	}
}

void MyAES::ShiftRow(uint8_t state[]) const
{
	static uint8_t temp[16];
	memcpy(temp, state, blockSize);
	for (int i = 0; i < blockSize; ++i)
	{
		int c = i % 4;
		state[i] = temp[(i + c * 4) % blockSize];
	}
}

void MyAES::InvShiftRow(uint8_t state[]) const
{
	static uint8_t temp[32];
	memcpy(temp, state, blockSize);
	for (int i = 0; i < blockSize; ++i)
	{
		int c = i % 4;
		state[i] = temp[(i + (blockSize - c * 4)) % blockSize];
	}
}

uint8_t MyAES::GFMul(int n, uint32_t u) const
{
	switch (n)
	{
	case 1:
		return u;
	case 2:
		return GFMul2(u);
	default:
		if (n % 2 == 1)
			return GFMul(n - 1, u) ^ u;
		else
			return GFMul2(GFMul(n / 2, u));
	}
}

uint8_t MyAES::GFMul2(uint32_t u)
{
	u <<= 1;
	if ((u & 0x100) != 0)
		u ^= 0x11B;
	return u;
}

void MyAES::MixColumn(uint8_t state[]) const
{
	static const int c[4][4] = {
		2, 3, 1, 1,
		1, 2, 3, 1,
		1, 1, 2, 3,
		3, 1, 1, 2 };

	//setting 8 rows is used for supporting the origin Rijndael Algorithm
	static uint8_t temp[8][4];
	memcpy(temp, state, blockSize);

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < Nb; ++j)
			state[j * 4 + i] =
			GFMul(c[i][0], temp[j][0]) ^
			GFMul(c[i][1], temp[j][1]) ^
			GFMul(c[i][2], temp[j][2]) ^
			GFMul(c[i][3], temp[j][3]);
}

void MyAES::InvMixColumn(uint8_t state[]) const
{
	static const int c[4][4] =
	{ 0x0e, 0x0b, 0x0d, 0x09,
	0x09, 0x0e, 0x0b, 0x0d,
	0x0d, 0x09, 0x0e, 0x0b,
	0x0b, 0x0d, 0x09, 0x0e };

	static uint8_t temp[8][4];
	memcpy(temp, state, blockSize);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < Nb; ++j)
			state[j * 4 + i] =
			GFMul(c[i][0], temp[j][0]) ^
			GFMul(c[i][1], temp[j][1]) ^
			GFMul(c[i][2], temp[j][2]) ^
			GFMul(c[i][3], temp[j][3]);
}

void MyAES::Round(uint8_t state[], const uint32_t roundKey[]) const
{
	ByteSub(state);

	ShiftRow(state);

	MixColumn(state);

	AddRoundKey(state, roundKey);
}

void MyAES::InvRound(uint8_t state[], const uint32_t roundKey[]) const
{
	AddRoundKey(state, roundKey);

	InvMixColumn(state);

	InvShiftRow(state);

	InvByteSub(state);
}

void MyAES::FinalRound(uint8_t state[], const uint32_t roundKey[]) const
{
	ByteSub(state);

	ShiftRow(state);

	AddRoundKey(state, roundKey);
}

void MyAES::InvFinalRound(uint8_t state[], const uint32_t roundKey[]) const
{
	AddRoundKey(state, roundKey);
	InvShiftRow(state);
	InvByteSub(state);
}

void MyAES::EncryptBlock(uint8_t state[])
{
	KeyExpansion(key, expandedKey);
	AddRoundKey(state, expandedKey);

	//round = 10, 12, 14
	for (int i = 1; i < round; ++i)
		// 4 * i = [4, 36], [4, 44], [4, 52]
		Round(state, expandedKey + Nb * i);

	// 4 * round = 40, 48, 56
	FinalRound(state, expandedKey + Nb * round);
}

void MyAES::DecryptBlock(uint8_t state[])
{
	KeyExpansion(key, expandedKey);

	InvFinalRound(state, expandedKey + Nb * round);

	for (int i = round - 1; i >= 1; --i)
	{
		InvRound(state, expandedKey + Nb * i);
	}
	AddRoundKey(state, expandedKey);
}

uint32_t MyAES::RotByte(uint32_t u)
{
	uint32_t temp = u << 24;
	return u >> 8 | temp;
}

uint32_t MyAES::SubByte(uint32_t u)
{
	uint32_t ret;
	uint8_t* pret = (uint8_t*)&ret;
	uint8_t* pu = (uint8_t*)&u;
	pret[0] = sBox[pu[0]];
	pret[1] = sBox[pu[1]];
	pret[2] = sBox[pu[2]];
	pret[3] = sBox[pu[3]];
	return ret;
}

void MyAES::KeyExpansion(const uint8_t key[], uint32_t W[]) const
{
	static const uint32_t Rcon[] = { 0x00,0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36 };
	uint32_t* pkey = (uint32_t*)key;
	for (int i = 0; i < Nk; ++i)
	{
		W[i] = pkey[i];
	}

	//for 256 bits: i in [8, 60)
	for (int i = Nk; i < Nb * (round + 1); ++i)
	{
		uint32_t temp = W[i - 1];
		if (i % Nk == 0)
			temp = SubByte(RotByte(temp)) ^ Rcon[i / Nk];
		else
			if (Nk == 8 && i % Nk == 4)
				temp = SubByte(temp);
		W[i] = W[i - Nk] ^ temp;
	}
}

size_t MyAES::Encrypt(uint8_t plain[], size_t len)
{
	size_t block = len / blockSize;
	size_t remain_len = len % blockSize;
	switch (mode)
	{
	case ECB:
	{
		//对于整块数据，逐块加密
		for (size_t i = 0; i < block; ++i)
		{
			EncryptBlock(plain + i * blockSize);
		}

		//剩余部分，先填0，再填充plain碎片部分
		//最后对remain_buf进行加密
		if (remain_len)
		{
			memset(remain_buf, 0, blockSize); //padding 0
			memcpy(remain_buf, plain + block * blockSize, remain_len);
			EncryptBlock(remain_buf);
		}
		return blockSize * block;
	}
	case CBC:
	{
		for (size_t i = 0; i < block; ++i)
		{
			//XOR with iv
			for (int n = 0; n < blockSize; ++n)
				(plain+i*blockSize)[n] ^= iv[n];
			EncryptBlock(plain + i * blockSize);

			//设置iv为当前encrypted block
			memcpy(iv, plain + i * blockSize, blockSize);
		}

		//对于剩余部分，将remain_buf填充后，全部与前一个块异或，再加密
		if (remain_len)
		{
			memset(remain_buf, 0, blockSize); //padding 0
			memcpy(remain_buf, plain + block * blockSize, remain_len);
			for (int n = 0; n < blockSize; ++n)
				remain_buf[n] ^= iv[n];
			EncryptBlock(remain_buf);
		}
		return blockSize * block;
	}
	case CTR:
	{
		//counter用于记录块编号，就算加密过程中不是整块，
		//只要末尾碎片进行了检测并写入，末尾的碎片就仍带有编号。
		//这样解密过程中的块编号仍然连续。
		size_t counter = 0;
		std::default_random_engine e;
		for (size_t i = 0; i < block; ++i,++counter)
		{
			//将序号送入random engine得到counter
			e.seed((uint32_t)counter);

			//借用iv用于存储counter
			uint32_t* p64 = (uint32_t*)iv;
			p64[0] = e();
			p64[1] = e();
			p64[2] = e();
			p64[3] = e();

			//加密
			EncryptBlock(iv);

			for (int j = 0; j < blockSize; ++j)
				plain[i * blockSize + j] ^= iv[j];
		}

		//对于剩余部分，将remain_buf填充后与加密后的counter进行异或
		if (remain_len)
		{
			memset(remain_buf, 0, blockSize); //padding 0
			memcpy(remain_buf, plain + block * blockSize, remain_len);

			//将序号送入random engine得到counter
			e.seed((uint32_t)counter);

			//借用iv用于存储counter
			uint32_t* p64 = (uint32_t*)iv;
			p64[0] = e();
			p64[1] = e();
			p64[2] = e();
			p64[3] = e();

			//加密
			EncryptBlock(iv);

			for (int j = 0; j < blockSize; ++j)
				remain_buf[j] ^= iv[j];
		}

		return block * blockSize;
	}
	case CFB1:
	{
		for (size_t i = 0; i < len; ++i)
		{
			bitset<8> p(plain[i]);
			//对于每字节，逐位进行处理
			for (size_t j = 0; j < 8; ++j)
			{
				EncryptBlock(iv);

				//p当前位与iv最高位异或
				p[j] = p[j] ^ (iv[0] >> 7);

				//iv左移1位
				for (int i = 0; i < 15; ++i)
				{
					iv[i] <<= 1;
					//当前字节最低位=下一字节最低位
					iv[i] |= ((int8_t)iv[i + 1] >= 0) ? 0 : 0x01;
				}
				iv[15] <<= 1;

				//iv最低位填上c
				iv[15] |= p[j];
			}
			//设置plain=E(cipher)
			plain[i] = (uint8_t)p.to_ulong();
		}
		return len;
	}
	case CFB8:
	{
		for (size_t i = 0; i < len; ++i)
		{
			EncryptBlock(iv);
			plain[i] ^= iv[0];

			//left shift iv
			//高位至低位copy，不会产生overlapping
			memcpy(iv, iv + 1, iv_bytes - 1);

			//the last one = cipher
			iv[iv_bytes - 1] = plain[i];
		}
		return len;
	}
	case OFB1:
	{
		uint8_t tempIv[16];
		for (size_t i = 0; i < len; ++i)
		{
			bitset<8> p(plain[i]);
			//对于每字节，逐位进行处理
			for (size_t j = 0; j < 8; ++j)
			{
				//暂存iv
				memcpy(tempIv, iv, 16);

				EncryptBlock(tempIv);

				//与tempIv最高位异或
				p[j] =p[j]^ ((int8_t)tempIv[0] < 0);

				//iv左移1位
				for (int i = 0; i < 15; ++i)
				{
					iv[i] <<= 1;
					//当前字节最低位=下一字节最低位
					iv[i] |= ((int8_t)iv[i + 1] < 0) ? 0x01 : 0x00;
				}
				iv[15] <<= 1;

				//iv最低位填上加密后iv最低位
				iv[15] |= tempIv[iv_bytes-1]&0x01;
			}
			//设置plain=E(cipher)
			plain[i] = (uint8_t)p.to_ulong();
		}
		return len;
	}
	case OFB8:
	{
		uint8_t tempIv[16];
		for (size_t i = 0; i < len; ++i)
		{
			//暂存iv
			memcpy(tempIv, iv, 16);

			EncryptBlock(tempIv);

			plain[i] ^= tempIv[0];

			//iv左移1B
			memcpy(iv, iv + 1, iv_bytes - 1);

			//iv最后一字节 = iv加密后的最后一字节
			iv[iv_bytes - 1] = tempIv[iv_bytes - 1];
		}
		return len;
	}
	}
	return 0;
}

void MyAES::Decrypt(uint8_t cipher[], size_t len)
{
	size_t block = len / blockSize;
	size_t remain_len = len % blockSize;

	switch (mode)
	{
	case ECB:
		for (size_t i = 0; i < block; ++i)
		{
			DecryptBlock(cipher + i * blockSize);
		}
		break;
	case CBC:
	{
		uint8_t iv_temp[16];
		for (size_t i = 0; i < block; ++i)
		{
			//暂存当前块，当前块结束后，将暂存的块用于下一个块的解密
			memcpy(iv_temp, cipher + i * blockSize, 16);

			DecryptBlock(cipher + i * blockSize);

			//异或IV
			for (int n = 0; n < blockSize; ++n)
				(cipher+i*blockSize)[n] ^= iv[n];

			//将暂存的块存入IV
			memcpy(iv, iv_temp, 16);
		}
		break;
	}

	case CTR:
	{
		size_t counter = 0;
		std::default_random_engine e;
		for (size_t i = 0; i < block; ++i,++counter)
		{
			//将序号送入random engine得到counter
			e.seed((uint32_t)counter);

			//借用iv用于存储counter
			uint32_t* p64 = (uint32_t*)iv;
			p64[0] = e();
			p64[1] = e();
			p64[2] = e();
			p64[3] = e();

			//
			EncryptBlock(iv);

			for (int j = 0; j < blockSize; ++j)
				cipher[i * blockSize + j] ^= iv[j];
		}
		break;
	}
	case CFB1:
	{
		for (size_t i = 0; i < len; ++i)
		{
			bitset<8> p(cipher[i]);
			//对于每字节，逐位进行处理
			for (size_t j = 0; j < 8; ++j)
			{
				EncryptBlock(iv);

				//保存cipher当前位
				uint8_t temp = p[j];

				//cipher当前位与iv最高位做异或
				p[j] = p[j] ^ (iv[0] >> 7);

				//iv左移1位
				for (int i = 0; i < 15; ++i)
				{
					iv[i] <<= 1;
					//当前字节最低位=下一字节最低位
					iv[i] |= ((int8_t)iv[i + 1] >= 0) ? 0 : 0x01;
				}
				iv[15] <<= 1;

				//iv最低位=cipher当前位
				iv[15] |= temp;
			}
			//设置plain=D(cipher)
			cipher[i] = (uint8_t)p.to_ulong();
		}
		break;
	}
	case CFB8:
	{
		for (size_t i = 0; i < len; ++i)
		{
			EncryptBlock(iv);
			unsigned char c = cipher[i];
			cipher[i] ^= iv[0];

			//left shift iv
			//高位至低位copy，不会产生overlapping
			memcpy(iv, iv + 1, iv_bytes - 1);

			//the last one = cipher
			iv[iv_bytes - 1] = c;
		}
		break;
	}
	case OFB1:
	{
		uint8_t tempIv[16];
		for (size_t i = 0; i < len; ++i)
		{
			bitset<8> p(cipher[i]);
			//对于每字节，逐位进行处理
			for (size_t j = 0; j < 8; ++j)
			{
				//暂存iv
				memcpy(tempIv, iv, 16);

				EncryptBlock(tempIv);

				//与tempIv最高位异或
				p[j] = p[j] ^ ((int8_t)tempIv[0] < 0);

				//iv左移1位
				for (int i = 0; i < 15; ++i)
				{
					iv[i] <<= 1;
					//当前字节最低位=下一字节最低位
					iv[i] |= ((int8_t)iv[i + 1] < 0) ? 0x01 : 0x00;
				}
				iv[15] <<= 1;

				//iv最低位填上加密后iv最低位
				iv[15] |= tempIv[iv_bytes - 1] & 0x01;
			}
			//设置cipher=E(plain)
			cipher[i] = (uint8_t)p.to_ulong();
		}
		break;
	}
	case OFB8:
	{
		uint8_t tempIv[16];
		for (size_t i = 0; i < len; ++i)
		{
			//暂存iv
			memcpy(tempIv, iv, 16);

			EncryptBlock(tempIv);

			cipher[i] ^= tempIv[0];

			//iv左移1B
			memcpy(iv, iv + 1, iv_bytes - 1);

			//iv最后一字节 = iv加密后的最后一字节
			iv[iv_bytes - 1] = tempIv[iv_bytes - 1];
		}
		break;
	}
	}
}