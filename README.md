# Encrypt AES

Author: Tom Willow

Origin url: https://github.com/tomwillow/Encrypt-AES

# 截图 Snapshot

![Here is a picture.](snapshot/snapshot.png)

# 特性 Features

- 实现完整的AES加密；The whole AES standard is achieved.
- 支持128位、196位、256位加密；Support 128-bits, 196-bits, 256-bits encryption.
- 分组模式支持ECB, CBC, CTR, CFB-1, CFB-8, OFB-1, OFB-8；Supported block cipher modes: ECB, CBC, CTR, CFB-1, CFB-8, OFB-1, OFB-8.
- 支持查表加速；Support speedup by checking table.
- 支持使用IV；Support using IV.
- 文件格式没有限制。File format is unlimited.

# 封装良好的C++ AES类 Well-encapsulated AES class in C++

输入密钥、密钥长度，位数（默认128），分组模式（默认ECB），IV，IV长度，直接调用 Encrypt / Decrypt 加密/解密即可。Input the key, size of key, bits(default 128), block cipher mode(default ECB), IV, size of IV, then call Encrypt/Decrypt.

任何不符合的条件都会抛出std::string类型的异常。In any unsupported situation,  std::string exception will be thrown.

多余的块长度处理方式详见源码。Please see the source code with the dealing method of the redundant / crippled last-one block.

例子 Example:

```c++
const unsigned char input[] = "123456789012345";
const unsigned char key[] = "1234567890123456";
unsigned char output[16];
memcpy(output, input, 16);

MyAES aes(key, 16, 128);
aes.Encrypt(output, 16); // encrypt at the same place
```

# 感谢 Thanks

gmult is refered from https://github.com/dhuertas/AES.

```
Advanced Encryption Standard
author Dani Huertas
email huertas.dani@gmail.com
Based on the document FIPS PUB 197
```

