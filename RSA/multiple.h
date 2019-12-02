#pragma once
#include <stdio.h>
	/**
	 * Basic limb type. Note that some calculations rely on unsigned overflow wrap-around of this type.
	 * As a result, only unsigned types should be used here, and the RADIX, HALFRADIX above should be
	 * changed as necessary. Unsigned integer should probably be the most efficient word type, and this
	 * is used by GMP for example.
	 */
	typedef unsigned int word;

	/**
	 * Structure for representing multiple precision integers. This is a base "word" LSB
	 * representation. In this case the base, word, is 2^32. Length is the number of words
	 * in the current representation. Length should not allow for trailing zeros (Things like
	 * 000124). The capacity is the number of words allocated for the limb data.
	 */
	typedef struct _bignum {
		int length;
		int capacity;
		word* data;
	} bignum;

	bignum* bignum_init();
	void bignum_deinit(bignum* b);

	void bignum_print(FILE* fp, bignum* b);

	void bignum_writebin(FILE* fp, bignum* b);

	void bignum_fromstring(bignum* b, const char* string);
	void bignum_fromint(bignum* b, unsigned int num);

	void bignum_subtract(bignum* result, bignum* b1, bignum* b2);
	void bignum_imultiply(bignum* source, bignum* add);
	void bignum_multiply(bignum* result, bignum* b1, bignum* b2);

	int bignum_less(bignum* b1, bignum* b2);

	void randPrime(int numDigits, bignum* result);

	void randExponent(bignum* phi, int n, bignum* result);

	void bignum_inverse(bignum* a, bignum* m, bignum* result);

	/**
	 * Save some frequently used bigintegers (0 - 10) so they do not need to be repeatedly
	 * created. Used as, NUMS[5] = bignum("5"), etc..
	 */
	extern bignum NUMS[11];

	bignum* encodeMessage(int len, int bytes, char* message, bignum* exponent, bignum* modulus);
	int* decodeMessage(int len, int bytes, bignum* cryptogram, bignum* exponent, bignum* modulus);

	void encodeToFile(FILE* fp,unsigned char* message, int len, int bytes, bignum* exponent, bignum* modulus);

	void decodeFromData(unsigned char* result,unsigned char* message, int len, int encodedBlockSize, int blockBytes, bignum* exponent, bignum* modulus);