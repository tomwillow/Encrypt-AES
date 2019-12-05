#pragma once
#include <stdint.h>

uint64_t GetDESKeyByUC(const unsigned char* s, int len);
uint64_t GetDESKeyByC(const char* s, int len);

//��keyд��s
//s�豣֤ǰ8B����д��
void WriteDESKey(uint64_t key, unsigned char* s);

//����
uint64_t Encrypt(uint64_t plain, uint64_t key);

//����
uint64_t Decrypt(uint64_t plain, uint64_t key);

//����
//���len����8�ı������������ֽ��ճ�8�ֽ�д��remain_buf
void EncryptData(unsigned char* plain, size_t len, unsigned char* remain_buf, size_t* remain_len, uint64_t key);

//����
//���len����8�ı�����������ֽ�������
void DecryptData(unsigned char* plain, size_t len, uint64_t key);