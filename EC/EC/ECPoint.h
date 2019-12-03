#pragma once

#include "Utils.h"
#include "GFPolynomial.h"

#include <functional>

// { x, y } where x, y in GF(2^m)
struct ECPoint {

#ifdef V1
	static const size_t						m = 163;
	static const uint32_t					A = 1;
	inline static const std::string			B = "5FF6108462A2DC8210AB403925E638A19C1455D21";
	inline static const std::vector<size_t> f_coefs = { 0, 3, 6, 7, 163 };
#elif defined V2
	static const size_t						m = 167;
	static const uint32_t					A = 1;
	inline static const std::string			B = "6EE3CEEB230811759F20518A0930F1A4315A827DAC";
	inline static const std::vector<size_t> f_coefs = { 0, 6, 167 };
#elif defined V3
	static const size_t						m = 173;
	static const uint32_t					A = 0;
	inline static const std::string			B = "108576C80499DB2FC16EDDF6853BBB278F6B6FB437D9";
	inline static const std::vector<size_t> f_coefs = { 0, 1, 2, 10, 173 };
#elif defined V7
	static const size_t						m = 257;
	static const uint32_t					A = 0;
	inline static const std::string			B = "1CEF494720115657E18F938D7A7942394FF9425C1458C57861F9EEA6ADBE3BE10";
	inline static const std::vector<size_t> f_coefs = { 0, 12, 257 };
#elif defined V9
	static const size_t						m = 367;
	static const uint32_t					A = 1;
	inline static const std::string			B = "43FC8AD242B0B7A6F3D1627AD5654447556B47BF6AA4A64B0C2AFE42CADAB8F93D92394C79A79755437B56995136";
	inline static const std::vector<size_t> f_coefs = { 0, 21, 367 };
#elif defined V10
	static const size_t						m = 431;
	static const uint32_t					A = 1;
	inline static const std::string			B = "03CE10490F6A708FC26DFE8C3D27C4F94E690134D5BFF988D8D28AAEAEDE975936C66BAC536B18AE2DC312CA493117DAA469C640CAF3";
	inline static const std::vector<size_t> f_coefs = { 0, 1, 3, 5, 431 };
#endif

	typedef GFPolynomial<m> gf_polynomial;
	typedef GFPolynomial<m>::gf_bits gf_bits;

	// y^2 + x * y = x^3 + a * x^2 + b mod f where a, b, x, y from GF(2^m)
	inline static const gf_bits f = bitset_from_coefs<m + 1>(f_coefs);
	inline static gf_polynomial
		a = gf_polynomial{ A, f },
		b = gf_polynomial{ hex_to_bin<m + 1>(B), f };

	gf_polynomial x, y;

	ECPoint() : x{ GFPolynomial<m>::gen_random_poly(f) }, y{ f } {
		for (int solutions = 0; ; x = GFPolynomial<m>::gen_random_poly(f)) {
			auto x_square = x * x;
			std::tie(solutions, y) = GFPolynomial<m>::solve_sqr_eq(x, x_square * x + a * x_square + b);
			if (solutions) {
				break;
			}
		}
	}

	ECPoint(const gf_polynomial& x_poly, const gf_polynomial& y_poly) : x{ x_poly }, y{ y_poly } { }

	inline operator bool() const {
		return isZero();
	}

	ECPoint operator+(const ECPoint& point) const {
		if (x != point.x) {
			return this->plus(point);
		}

		if (y == point.y) {
			return this->doubled();
		}

		return zero();
	}

	ECPoint operator+=(const ECPoint& point) {
		*this = *this + point;
	}

	template<int16_t s>
	ECPoint operator*(const std::bitset<s>& times_bits) const {
		auto result = zero();
		auto m = s;
		while (!times_bits[--m] && m);

		if (m || times_bits[m]) {
			for (; m >= 0; m--) {
				result = result.doubled();
				if (times_bits[m]) {
					result = result.plus(*this);
				}
			}
		}

		return result;
	}

	ECPoint plus(const ECPoint& point) const {
		if (this->isZero()) {
			return point;
		}

		if (point.isZero()) {
			return *this;
		}

		auto lambda = (y + point.y) * ~(x + point.x);
		auto new_x = lambda * lambda + lambda + x + point.x + a;
		auto new_y = lambda * (x + new_x) + new_x + y;

		return { std::move(new_x), std::move(new_y) };
	}

	ECPoint doubled() const {
		if (this->isZero()) {
			return *this;
		}

		auto mu = x + y * ~x;
		auto new_x = mu * mu + mu + a;
		auto new_y = x * x + (mu + gf_polynomial{ 1, f }) * new_x;

		return { std::move(new_x), std::move(new_y) };
	}

	inline bool isZero() const {
		return !x && !y;
	}

	static inline ECPoint zero() {
		return ECPoint{ { 0b0, f },{ 0b0, f } };
	}
};
