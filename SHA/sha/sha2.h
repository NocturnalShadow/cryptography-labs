#pragma once

#include <cstdint>
#include <array>

#include "utils.h"

namespace sha2 {
	typedef uint32_t word;

	constexpr size_t rounds_count = 64;
	constexpr size_t digest_bit_size = 256;
	constexpr size_t digest_words_count = digest_bit_size / (sizeof(word) * 8);

	constexpr std::array<word, rounds_count> k = {
		0x428A2F98u, 0x71374491u, 0xB5C0FBCFu, 0xE9B5DBA5u, 0x3956C25Bu, 0x59F111F1u, 0x923F82A4u, 0xAB1C5ED5u,
		0xD807AA98u, 0x12835B01u, 0x243185BEu, 0x550C7DC3u, 0x72BE5D74u, 0x80DEB1FEu, 0x9BDC06A7u, 0xC19BF174u,
		0xE49B69C1u, 0xEFBE4786u, 0x0FC19DC6u, 0x240CA1CCu, 0x2DE92C6Fu, 0x4A7484AAu, 0x5CB0A9DCu, 0x76F988DAu,
		0x983E5152u, 0xA831C66Du, 0xB00327C8u, 0xBF597FC7u, 0xC6E00BF3u, 0xD5A79147u, 0x06CA6351u, 0x14292967u,
		0x27B70A85u, 0x2E1B2138u, 0x4D2C6DFCu, 0x53380D13u, 0x650A7354u, 0x766A0ABBu, 0x81C2C92Eu, 0x92722C85u,
		0xA2BFE8A1u, 0xA81A664Bu, 0xC24B8B70u, 0xC76C51A3u, 0xD192E819u, 0xD6990624u, 0xF40E3585u, 0x106AA070u,
		0x19A4C116u, 0x1E376C08u, 0x2748774Cu, 0x34B0BCB5u, 0x391C0CB3u, 0x4ED8AA4Au, 0x5B9CCA4Fu, 0x682E6FF3u,
		0x748F82EEu, 0x78A5636Fu, 0x84C87814u, 0x8CC70208u, 0x90BEFFFAu, 0xA4506CEBu, 0xBEF9A3F7u, 0xC67178F2u
	};

	void process_block(const word block[16], std::array<word, 8>& state) {
		auto w = new word[rounds_count];		// schedule std::array
		memcpy(w, block, 16 * sizeof(word));

		word s0, s1;
		word a, b, c, d, e, f, g, h;

		for (int i = 16; i < rounds_count; ++i) {
			s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
			s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		}

		a = state[0]; 
		b = state[1]; 
		c = state[2]; 
		d = state[3];
		e = state[4]; 
		f = state[5]; 
		g = state[6]; 
		h = state[7];

		word sig0, ma, sig1, ch, temp;
		for (int i = 0; i < rounds_count; ++i) {
			sig0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
			ma = (a & b) ^ (a & c) ^ (b & c);

			sig1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
			ch = (e & f) ^ (~e & g);

			temp = h + sig1 + ch + k[i] + w[i];
			h = g; g = f; f = e; e = d + temp;
			d = c; c = b; b = a; a = temp + sig0 + ma;
		}

		state[0] += a;
		state[1] += b;
		state[2] += c;
		state[3] += d;
		state[4] += e;
		state[5] += f;
		state[6] += g;
		state[7] += h;
	}

	std::array<word, digest_words_count> hash(const char* message, uint64_t message_size) {
		// Initial state
		std::array<word, 8> state = {
			0x6A09E667u,
			0xBB67AE85u,
			0x3C6EF372u,
			0xA54FF53Au,
			0x510E527Fu,
			0x9B05688Cu,
			0x1F83D9ABu,
			0x5BE0CD19u
		};

		// Pre-Processing
		auto words_count = message_size / 4 + 1,														// words count for (message + 1-bit)
			reserved_words_count = words_count + 2,														// words count for (message + 1-bit + message length), where message length is 64 bits
			padded_words_count = (reserved_words_count / 16 + bool(reserved_words_count % 16)) * 16;	// words count for (message bits + 1-bit + 0-bits + message length)

		auto padded_message = new word[padded_words_count]();											// padded message bit-length == 0 (mod 512)

		for (uint64_t i = 0u; i < message_size; i++) {													// original message bits
			padded_message[i / 4] |= word(message[i]) << (8 * (3 - i % 4));
		}
		
		padded_message[words_count - 1] |= 1 << (7 + 8 * (3 - message_size % 4));						// set 1-bit
		padded_message[padded_words_count - 2] = (message_size * 8 >> 32) & 0xFFFFFFFF;					// set last 64 bit to message length
		padded_message[padded_words_count - 1] = (message_size * 8) & 0xFFFFFFFF;

		// Processing
		for (auto i = 0u; i < padded_words_count; i += 16) {
			process_block(padded_message + i, state);
		}

		delete[] padded_message;

		return state;
	}
}