#pragma once


//-----------------------------------------------------------------------------------------------
void AddNoise(uint32& previous, uint32 seed)
{
	const uint32 BIT_NOISE1 = 0xB5297A4D; // 0b1011'0101'0010'1001'0111'1010'0100'1101;
	const uint32 BIT_NOISE2 = 0x68E31DA4; // 0b0110'1000'1110'0011'0001'1101'1010'0100;
	const uint32 BIT_NOISE3 = 0x1B56C4E9; // 0b0001'1011'0101'0110'1100'0100'1110'1001;

	uint32& mangledBits = previous;
	mangledBits *= BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 8);
	mangledBits += BIT_NOISE2;
	mangledBits ^= (mangledBits << 8);
	mangledBits *= BIT_NOISE3;
	mangledBits ^= (mangledBits >> 8);
}