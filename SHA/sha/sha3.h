#pragma once

#include <cstdint>
#include <array>

#include "utils.h"

namespace sha3 {
	typedef uint64_t word;

	constexpr size_t state_bit_size = 1600; // Keccak-1600
	constexpr size_t digest_bit_size = 512;	// SHA3-512
	constexpr size_t block_bit_size = 576;	// for Kaccak-1600

	constexpr size_t block_byte_size = block_bit_size / 8;

	constexpr size_t state_words_count = state_bit_size / (sizeof(word) * 8);
	constexpr size_t digest_words_count = digest_bit_size / (sizeof(word) * 8);
	constexpr size_t block_words_count = block_bit_size / (sizeof(word) * 8);

	constexpr size_t rounds_count = 24;	// rounds count - 24 for Keccak-1600

	constexpr word rc[rounds_count] = {			// round constants
		0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
		0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
		0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
		0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
		0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
		0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
		0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
		0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
	};

	constexpr int r[5][5] = {					// word rotation table
		{ 0, 36, 3, 41, 18 },
	{ 1, 44, 10, 45, 2 },
	{ 62, 6, 43, 15, 61 },
	{ 28, 55, 25, 21, 56 },
	{ 27, 20, 39, 8, 14 }
	};

	void process_block(word A[5][5]) {
		for (int round = 0; round < rounds_count; round++) {
			// Step Theta
			word C[5];
			for (int i = 0; i < 5; i++) {
				C[i] = A[i][0] ^ A[i][1] ^ A[i][2] ^ A[i][3] ^ A[i][4];
			}

			word D;
			for (int i = 0; i < 5; i++) {
				D = C[(i + 4) % 5] ^ rotl(C[(i + 1) % 5], 1);

				for (int j = 0; j < 5; j++) {
					A[i][j] ^= D;
				}
			}

			// Step Rho and Pi
			word B[5][5];
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					B[j][(2 * i + 3 * j) % 5] = rotl(A[i][j], r[i][j]);
				}
			}

			// Step Chi
			for (int i = 0; i < 5; i++) {
				for (int j = 0; j < 5; j++) {
					A[i][j] = B[i][j] ^ ((~B[(i + 1) % 5][j]) & B[(i + 2) % 5][j]);
				}
			}

			// Step Iota
			A[0][0] ^= rc[round];
		}
	}

	std::array<word, digest_words_count> hash(const char* message, uint64_t message_size) {
		// Pre-processing (padding)
		auto reserved_message_size = message_size + 2,																								// bytes count for (message + 0x06 byte + 0x80 byte)
			reserved_words_count = reserved_message_size / sizeof(word) + bool(reserved_message_size % sizeof(word)),								// words count for (message + 0x06 byte + 0x80 byte)
			padded_words_count = (reserved_words_count / block_words_count + bool(reserved_words_count % block_words_count)) * block_words_count;	// words count for (message + 0x06 byte + 0 bytes + 0x80 byte)

		auto padded_message = new word[padded_words_count]();
		auto padded_message_bytes = (char*)padded_message;
		for (uint64_t i = 0u; i < message_size; i++) {		// original message bytes
			padded_message_bytes[i] = message[i];
		}

		padded_message_bytes[message_size] = 0x06;
		padded_message_bytes[padded_words_count * sizeof(word) - 1] = 0x80;

		// Processing
		word state[5][5]{};
		for (auto i = 0u; i < padded_words_count; i += block_words_count) {
			for (int j = 0; j < block_words_count; j++) {
				state[j % 5][j / 5] ^= padded_message[j];
			}

			process_block(state);
		}

		delete[] padded_message;

		std::array<word, digest_words_count> result;

		auto state_words = (word*)state;
		for (auto i = 0u; i < digest_words_count; i++) {
			result[i] = swap_bytes(state[i % 5][i / 5]);
		}

		return std::move(result);
	}
}
