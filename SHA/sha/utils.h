#pragma once

#include <bitset>

// Rotate right
template<typename T>
T rotr(T value, int positions) {
	return (value >> positions) | (value << (8 * sizeof(T) - positions));
}

// Rotate left
template<typename T>
T rotl(T value, int positions) {
	return (value << positions) | (value >> (8 * sizeof(T) - positions));
}

// Convert to bitset
template<typename T>
auto to_bitset(T value) {
	return std::bitset<8 * sizeof(T)>(value);
}

template<typename T>
T swap_bytes(T value)
{
	T result;
	for (int i = 0; i < sizeof(T); i++) {
		result = (result << 8) | (value >> 8 * i) & 0xFF;
	}

	return result;
}

template<typename T>
auto measure_exec_time(T action) {
	auto before = std::chrono::high_resolution_clock::now();
	action();
	auto after = std::chrono::high_resolution_clock::now();

	return std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
}

std::tuple<char*, long> read_file(const char* file_name) {
	FILE* file;
	if (fopen_s(&file, file_name, "rb")) {
		return { nullptr, 0 };
	}

	fseek(file, 0, SEEK_END);
	auto file_size = ftell(file);       
	auto buffer = (char*) malloc(file_size);
	rewind(file);
	fread(buffer, file_size, 1, file);
	fclose(file);

	return { buffer, file_size };
}