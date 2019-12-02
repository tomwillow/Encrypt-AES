#pragma once
#include <stdint.h>

uint64_t GetDESKeyByUC(const unsigned char* s, int len);
uint64_t GetDESKeyByC(const char* s, int len);

void WriteDESKey(uint64_t key, unsigned char* s);

//º”√‹
uint64_t Encrypt(uint64_t plain, uint64_t key);

//Ω‚√‹
uint64_t Decrypt(uint64_t plain, uint64_t key);

void EncryptData(unsigned char* plain, size_t len, unsigned char* remain_buf, size_t* remain_len, uint64_t key);

void DecryptData(unsigned char* plain, size_t len, uint64_t key);