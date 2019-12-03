#pragma once

#include <string>
#include <chrono>
#include <bitset>
#include <limits>
#include <random>
#include <vector>

template<typename T>
auto measure_exec_time(T action) {
	auto before = std::chrono::high_resolution_clock::now();
	action();
	auto after = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
}

const char* hex_to_bin(char c) {
	switch (toupper(c)) {
	case '0': return "0000";
	case '1': return "0001";
	case '2': return "0010";
	case '3': return "0011";
	case '4': return "0100";
	case '5': return "0101";
	case '6': return "0110";
	case '7': return "0111";
	case '8': return "1000";
	case '9': return "1001";
	case 'A': return "1010";
	case 'B': return "1011";
	case 'C': return "1100";
	case 'D': return "1101";
	case 'E': return "1110";
	case 'F': return "1111";
	}
}

template<size_t m>
std::bitset<m> hex_to_bin(const std::string& hex) {
	std::string result;
	for (auto i = 0u; i != hex.length(); i++) {
		result += hex_to_bin(hex[i]);
	}

	return std::bitset<m>{ result };
}

template<size_t m>
std::bitset<m> bitset_from_coefs(std::vector<size_t> non_zero_coefs) {
	std::bitset<m> result;
	for (auto coef : non_zero_coefs) {
		result.set(coef);
	}

	return result;
};

template<size_t m>
std::bitset<m> gen_random_bitset() {
	static auto generate_rand_int = std::bind(
		std::uniform_int_distribution<>(
			std::numeric_limits<int>::min(),
			std::numeric_limits<int>::max()),
		std::default_random_engine());

	std::bitset<m> bits;
	for (int i = 0; i < (m + 1) / sizeof(int) + 1; i++) {
		bits <<= sizeof(int);
		bits ^= std::bitset<m> { (uint64_t) generate_rand_int() };
	}

	return std::move(bits);
}