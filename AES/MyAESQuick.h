#pragma once
#include "MyAES.h"
class MyAESQuick :
	public MyAES
{
private:
	void QuickEncryptPrepare();
protected:
	virtual void EncryptBlock(uint8_t state[]) override;
	virtual void DecryptBlock(uint8_t state[]) override;

	virtual uint8_t GFMul(int n,uint32_t u)const override;
public:
	MyAESQuick(const uint8_t key[], int len, int bits = 128, AESMode mode = ECB, const uint8_t iv[] = nullptr, int iv_len = 0) :MyAES(key, len, bits,mode,iv,iv_len)
	{
		QuickEncryptPrepare();
	}
};

