
#include <iostream>
#include <cassert>
#include <fstream>
#include <iterator>
#include <vector>
#include <chrono>

#include "utils.h"
#include "sha2.h"
#include "sha3.h"


void test_utils() {
	assert(0b10000000000000000000000000000000 == rotr((sha2::word) 1, 1));
	assert(0b01000000000000000000000000000000 == rotr((sha2::word) 1, 2));
	assert(0b00000000000000000000000000000010 == rotr((sha2::word) 8, 2));
	assert(0b10000000000000000000000000000000 == rotr((sha2::word) 8, 4));
}

void test_sha2_256() {
	std::string message = "The quick brown fox jumps over the lazy dog";
	std::array<sha2::word, sha2::digest_words_count>
		digest_actual = sha2::hash(message.data(), message.length()),
		digest_expected = {
			0xD7A8FBB3, 0x07D78094, 0x69CA9ABC, 0xB0082E4F,
			0x8D5651E4, 0x6D3CDB76, 0x2D02D0BF, 0x37C9E592
		};

	assert(digest_actual == digest_expected);

	message = "The quick brown fox jumps over the lazy cog";
	digest_actual = sha2::hash(message.data(), message.length());
	digest_expected = {
		0xE4C4D8F3, 0xBF76B692, 0xDE791A17, 0x3E053211,
		0x50F7A345, 0xB46484FE, 0x427F6ACC, 0x7ECC81BE
	};

	assert(digest_actual == digest_expected);

	message = "";
	digest_actual = sha2::hash(message.data(), message.length());
	digest_expected = {
		0xe3b0c442, 0x98fc1c14, 0x9afbf4c8, 0x996fb924,
		0x27ae41e4, 0x649b934c, 0xa495991b, 0x7852b855
	};

	assert(digest_actual == digest_expected);

	std::cout << "test_sha2_256: SUCCESS" << std::endl;
}

void test_sha3_512() {
	std::string message = "The quick brown fox jumps over the lazy dog";
	std::array<sha3::word, sha3::digest_words_count>
		digest_actual = sha3::hash(message.data(), message.length()),
		digest_expected = {
			0x01dedd5de4ef1464, 0x2445ba5f5b97c15e, 0x47b9ad931326e4b0, 0x727cd94cefc44fff,
			0x23f07bf543139939, 0xb49128caf436dc1b, 0xdee54fcb24023a08, 0xd9403f9b4bf0d450
		};

	assert(digest_actual == digest_expected);

	message = "The quick brown fox jumps over the lazy cog";
	digest_actual = sha3::hash(message.data(), message.length());
	digest_expected = {
		0x28e361fe8c56e617, 0xcaa56c28c7c36e5c, 0x13be552b77081be8, 0x2b642f08bb7ef085, 
		0xb9a81910fe982693, 0x86b9aacfd2349076, 0xc9506126e198f6f6, 0xad44c12017ca77b1
	};

	assert(digest_actual == digest_expected);

	message = "";
	digest_actual = sha3::hash(message.data(), message.length());
	digest_expected = {
		0xa69f73cca23a9ac5, 0xc8b567dc185a756e, 0x97c982164fe25859, 0xe0d1dcc1475c80a6, 
		0x15b2123af1f5f94c, 0x11e3e9402c3ac558, 0xf500199d95b6d3e3, 0x01758586281dcd26
	};

	assert(digest_actual == digest_expected);

	std::cout << "test_sha3_512: SUCCESS" << std::endl;
}

void test_sha2_perf() {
	auto data_file = "test_data.bin";
	auto [data, data_size] = read_file(data_file);
	auto duration = measure_exec_time([&] {
		auto digest = sha2::hash(data, data_size);
	});
	std::cout << "File: " << data_file << " - " << data_size / pow(1024, 2) << "MB" << "\n";
	std::cout << "SHA2-256 generation: " << duration / 1000 << "." << duration % 1000 << " ms" << std::endl;
}

void test_sha3_perf() {
	auto data_file = "test_data.bin";
	auto[data, data_size] = read_file(data_file);
	auto duration = measure_exec_time([&] {
		auto digest = sha3::hash(data, data_size);
	});
	std::cout << "File: " << data_file << " - " << data_size / pow(1024, 2) << "MB" << "\n";
	std::cout << "SHA3-512 generation: " << duration / 1000 << "." << duration % 1000 << " ms" << std::endl;
}

int main() {
	test_utils();
	test_sha2_256();
	test_sha3_512();

#ifndef _DEBUG
	test_sha2_perf();
	test_sha3_perf();
#endif

	getchar();
}