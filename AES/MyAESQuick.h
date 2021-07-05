#pragma once
#include "MyAES.h"
class MyAESQuick :
	public MyAES
{
private:
	//准备函数，主要用于在加解密前调用 KeyExpansion 将 ExpandedKey 计算好
	void Prepare();
protected:

	//加速版块处理函数，与原版唯一区别就是开头没有调用 KeyExpansion 函数
	virtual void EncryptBlock(uint8_t state[]) override;
	virtual void DecryptBlock(uint8_t state[]) override;

	//重载 GFMul 以实现查表加速
	virtual uint8_t GFMul(int n,uint32_t u)const override;
public:
	MyAESQuick(const uint8_t key[], int len, int bits = 128, AESMode mode = ECB, const uint8_t iv[] = nullptr, int iv_len = 0) :MyAES(key, len, bits,mode,iv,iv_len)
	{
		Prepare();
	}
};

