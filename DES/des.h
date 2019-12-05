#pragma once
#include <stdint.h>

uint64_t GetDESKeyByUC(const unsigned char* s, int len);
uint64_t GetDESKeyByC(const char* s, int len);

//将key写入s
//s需保证前8B可以写入
void WriteDESKey(uint64_t key, unsigned char* s);

//加密
uint64_t Encrypt(uint64_t plain, uint64_t key);

//解密
uint64_t Decrypt(uint64_t plain, uint64_t key);

//加密
//如果len不是8的倍数，余数部分将凑成8字节写入remain_buf
void EncryptData(unsigned char* plain, size_t len, unsigned char* remain_buf, size_t* remain_len, uint64_t key);

//解密
//如果len不是8的倍数，多出部分将被忽略
void DecryptData(unsigned char* plain, size_t len, uint64_t key);