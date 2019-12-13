
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <assert.h>

#include "ScopeTime.h"
#include "MyAES.h"
#include "MyAESQuick.h"

using namespace std;


string GetModeName(MyAES::AESMode mode)
{
	switch (mode)
	{
	case MyAES::ECB:return "ECB";
	case MyAES::CBC:return "CBC";
	case MyAES::CTR:return "CTR";
	case MyAES::CFB1:return "CFB1";
	case MyAES::CFB8:return "CFB8";
	case MyAES::OFB1:return "OFB1";
	case MyAES::OFB8:return "OFB8";
	}
}

void printSep()
{
	cout << string(20, '-') << endl;
}

void print(unsigned char* state, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		if (i % 16 == 0) printf(" ");
		printf("%02x", state[i]);
	}
}

string str2hexStr(unsigned char str[], int len)
{
	string ret;
	for (int i = 0; i < len; ++i)
	{
		char temp[3];
		sprintf_s(temp,3, "%02x", str[i]);
		ret += temp;
	}
	return ret;
}

void hexStr2str(unsigned char* result, const unsigned char hex[], int len)
{
	int j = 0;
	bool low = true;
	unsigned char now;
	for (int i = 0; i < len; ++i)
	{
		char c = toupper(hex[i]);
		if (c == 0)
			return;
		if (c >= '0' && c <= '9')
			c = c - '0';
		else
			if (c >= 'A' && c <= 'F')
				c = c - 'A' + 10;
			else
			{
				printf("unexpected char: %c[%X]", c, c);
				return;
			}

		if (low)
			now = c << 4;
		else
		{
			now |= c;
			result[j++] = now;
		}
		low = !low;
	}
}

void testSpeed()
{
	const unsigned char input[] = "123456789012345";
	const unsigned char key[] = "1234567890123456";
	const unsigned char iv[] = "1234567890123456";

	unsigned char input2[16];

	int testcount = 100000;

	cout << "Testing speed..." << endl;
	std::this_thread::sleep_for(1s);

	{
		memcpy(input2, input, 16);
		int i = testcount;
		ScopeTime st;
		MyAES aes(key, 16, 128);
		while (i--)
			aes.Encrypt(input2, 16);
		cout << "My implemention:" << st.elapsed() << endl;
		cout << "cipher:"; print(input2, 16); cout << endl << endl;
	}
	{
		memcpy(input2, input, 16);
		int i = testcount;
		ScopeTime st;
		MyAESQuick aes(key, 16, 128);
		while (i--)
			aes.Encrypt(input2, 16);
		cout << "My implemention(quick ver.):" << st.elapsed() << endl;
		cout << "cipher:"; print(input2, 16); cout << endl << endl;
	}
	{
		memcpy(input2, input, 16);
		int i = testcount;
		ScopeTime st;
		MyAESQuick aes(key, 16, 128, MyAES::CBC, iv, 16);
		while (i--)
			aes.Encrypt(input2, 16);
		cout << "CBC:" << st.elapsed() << endl;
		cout << "cipher:"; print(input2, 16); cout << endl << endl;
	}
	{
		memcpy(input2, input, 16);
		int i = testcount / 100;
		ScopeTime st;
		MyAESQuick aes(key, 16, 128, MyAES::CFB1, iv, 16);
		while (i--)
			aes.Encrypt(input2, 16);
		cout << "CFB1:" << st.elapsed() << " *100" << endl;
		cout << "cipher:"; print(input2, 16); cout << endl << endl;
	}
	{
		memcpy(input2, input, 16);
		int i = testcount / 10;
		ScopeTime st;
		MyAESQuick aes(key, 16, 128, MyAES::CFB8, iv, 16);
		while (i--)
			aes.Encrypt(input2, 16);
		cout << "CFB8:" << st.elapsed() << " *10" << endl;
		cout << "cipher:"; print(input2, 16); cout << endl << endl;
	}
}

void test_inner(MyAES::AESMode mode,int bits,unsigned char plain[],int plain_len,unsigned char key[],int key_len,unsigned char iv[]=nullptr,int iv_len=0)
{
	printSep();
	cout << "bits:" <<bits<< endl;
	cout << "plain hex:"; print(plain, plain_len); cout << endl;
	cout << "plain:"<<plain << endl;

	cout << "key:"; print(key, key_len); cout << endl;

	//mode
	cout << "mode:" << GetModeName(mode) << endl;

	//cipher mine
	MyAES aes(key, key_len, bits, mode, iv, 16);
	int dealed = aes.Encrypt(plain, plain_len);
	cout << "My cipher    :"; print(plain, dealed);

	//decrypt
	bool hadPadding = plain_len % aes.GetBlockSize();
	if (hadPadding)
		print(aes.remain_buf, aes.GetBlockSize());
 cout << endl;

	//
	size_t decrypted_len = dealed + (hadPadding ? aes.GetBlockSize() : 0);
	unsigned char* decrypted = new unsigned char[decrypted_len];

	//
	memcpy(decrypted, plain, dealed);
	if (hadPadding)
		memcpy(decrypted + dealed, aes.remain_buf, aes.GetBlockSize());

	MyAESQuick aes2(key, key_len, bits, mode, iv, 16);
	aes2.Decrypt(decrypted, decrypted_len);
	cout << "after decrypt:"; print(decrypted, decrypted_len); cout << endl;
	cout <<"decrypted result:"<< decrypted << endl;

	memcpy(plain, decrypted, plain_len);

	delete[] decrypted;
	printSep();
}

void testPlain()
{
	cout << "Test Plain:" << endl;
	system("pause");

	unsigned char plain[17] = "abcdefghij123456";
	unsigned char key[33] = "12345678901234567890123456789012";
	unsigned char iv[17] = "1234567890123456";
	vector<int> vecBits{ 128,192,256 };
	vector<MyAES::AESMode> vecMode{ MyAES::ECB, MyAES::CBC, MyAES::CTR, MyAES::CFB1, MyAES::CFB8, MyAES::OFB1, MyAES::OFB8 };

	for (auto bits:vecBits)
	for (auto mode : vecMode)
	{
		unsigned char tempPlain[17];
		memcpy(tempPlain, plain, 16);
		test_inner(mode,bits, tempPlain, 16, key, bits/8, iv, 16);

		tempPlain[16] = 0;
		string s1((char*)plain);
		string s2((char*)tempPlain);
		if (s1 == s2)
		{
			cout << "passed" << endl;
			system("color 02");
		}
		else
		{
			cout << "error" << endl;
			system("color 04");
		}
		system("pause");
	}
}

void testHex()
{
	vector<string> vec { "6bc1bee22e409f96e93d7e117393172a",
		"ae2d8a571e03ac9c9eb76fac45af8e51",
		"30c81c46a35ce411e5fbc1191a0a52ef",
		"f69f2445df4f9b17ad2b417be66c3710" };

	vector<string> expected{ "3ad77bb40d7a3660a89ecaf32466ef97",
	"f5d3d58503b9699de785895a96fdbaaf",
	"43b1cd7f598ece23881b00e3ed030688",
	"7b0c785e27e8ad3f8223207104725dd4" };

	const unsigned char keyHex[] = "2b7e151628aed2a6abf7158809cf4f3c";

	for (size_t i=0;i<vec.size();++i)
	{
		unsigned char plain[16];
		unsigned char key[16];
		hexStr2str(plain, (unsigned char*)vec[i].c_str(), vec[i].length());
		hexStr2str(key, keyHex, 32);
		test_inner(MyAES::ECB,128, plain, 16, key, 16);

		string ret = str2hexStr(plain,16);
		assert(ret == expected[i]);
	}
}

void test()
{
	testPlain();
}
