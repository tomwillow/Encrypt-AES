#include "MyAESQuick.h"

#include "gmult.h"

uint8_t MyAESQuick::GFMul(int n, uint32_t u) const
{
	return gmult_aes[n][u];
}

void MyAESQuick::EncryptBlock(uint8_t state[])
{
	AddRoundKey(state, expandedKey);

	//round = 10, 12, 14
	for (int i = 1; i < round; ++i)
		// 4 * i = [4, 36], [4, 44], [4, 52]
		Round(state, expandedKey + 4 * i);

	// 4 * round = 40, 48, 56
	FinalRound(state, expandedKey + 4 * round);
}

void MyAESQuick::DecryptBlock(uint8_t state[])
{

	InvFinalRound(state, expandedKey + Nb * round);

	for (int i = round - 1; i >= 1; --i)
	{
		InvRound(state, expandedKey + Nb * i);
	}
	AddRoundKey(state, expandedKey);
}

//for 128 bits: size of state = 16
//for 256 bits: size of state = 32
void MyAESQuick::Prepare()
{
	KeyExpansion(key, expandedKey);
}