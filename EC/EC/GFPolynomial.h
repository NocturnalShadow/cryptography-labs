#pragma once

#include "Utils.h"

#include <vector>
#include <bitset>
#include <iostream>
#include <functional>

#include <assert.h>

template<size_t m>
struct GFPolynomial {
	static const size_t s = m + 1;
	typedef std::bitset<s> gf_bits;	// b0*x^0 + b1*x^1 + b2*x^2 + ... bm*x^m 
								// (*) in std::bitset least significant bit is first

	gf_bits bits;		// coefficients of the polynomial over GF2
	const gf_bits& p;	// irreducible polynomial of degree m over GF2

	GFPolynomial(const gf_bits& p) : p{ p } { }
	GFPolynomial(gf_bits bits, const gf_bits& p) : bits{ bits }, p{ p } { }

	GFPolynomial& operator=(const GFPolynomial& poly) {
		assert(&p == &poly.p);
		bits = poly.bits;
		return *this;
	}

	GFPolynomial operator+(const GFPolynomial& poly) const {
		assert(&p == &poly.p);
		return { bits ^ poly.bits, p };
	}

	GFPolynomial operator-(const GFPolynomial& poly) const {
		return *this + poly;
	}

	GFPolynomial operator*(const GFPolynomial& poly) const {
		assert(&p == &poly.p);
		return { multiply(bits, poly.bits, p), p };
	}

	GFPolynomial operator~() const {
		gf_bits inverse,
			next_inverse = 0b1,
			quotient,
			remainder = p,
			next_remainder = bits;

		while (next_remainder.any()) {
			auto prev_remainder = next_remainder;
			std::tie(quotient, next_remainder) = divide(remainder, next_remainder);
			remainder = std::move(prev_remainder);

			auto prev_inverse = next_inverse;
			next_inverse = inverse ^ multiply(quotient, next_inverse, p);
			inverse = std::move(prev_inverse);
		}

		return { inverse, p };
	}

	bool operator==(const GFPolynomial& poly) const {
		assert(&p == &poly.p);
		return bits == poly.bits;
	}

	bool operator!=(const GFPolynomial& poly) const {
		assert(&p == &poly.p);
		return bits != poly.bits;
	}

	operator bool() const {
		return bits.any();
	}

	friend std::ostream& operator<<(std::ostream& os, const GFPolynomial& poly) {
		return os << poly.bits;
	}

	size_t degree() const {
		return degree(bits);
	}

	GFPolynomial trace() const {
		auto t = bits;
		for (auto i = 1u; i < m; i++) {
			t = multiply(t, t, p) ^ bits;
		}
		return { t, p };
	}

	GFPolynomial half_trace() const {
		auto t = bits;
		for (auto i = 0u; i < (m - 1) / 2; i++) {
			t = multiply(t, t, p);
			t = multiply(t, t, p) ^ bits;
		}
		return { t, p };
	}

	static size_t degree(const gf_bits& bits) {
		for (auto i = s; i--; ) {
			if (bits[i]) {
				return i + 1;
			}
		}

		return 0;
	}

	static gf_bits multiply(const gf_bits& a, const gf_bits& b, const gf_bits& p) {
		// Simple polynomial multiplication
		// GFPolynomial<m> * GFPolynomial<m> = GFPolynomial<2 * m> = product ++ overflow
		gf_bits product;
		gf_bits overflow;

		for (auto i = 0u; i < s; i++) {
			if (b[i]) {
				product ^= a << i;
				overflow ^= a >> s - i;
			}
		}

		// Modulo division by irreducible polynomial p
		if (overflow.any()) {
			for (auto i = s + 1; i-- > 1; ) {
				if (overflow[i - 1]) {
					overflow ^= p >> s - i;
					product ^= p << i;
				}
			}
		}

		if (product[s - 1]) {
			product ^= p;
		}

		return std::move(product);
	}

	static std::tuple<gf_bits, gf_bits> divide(const gf_bits& divident, const gf_bits& divisor) {
		auto dividend_degree = degree(divident),
			divisor_degree = degree(divisor);

		assert(divisor_degree);

		if (dividend_degree < divisor_degree) {
			return { gf_bits{}, divident };
		}

		auto bound = divisor_degree - 1;
		gf_bits quotient, remainder = divident;
		for (auto i = dividend_degree; i-- > bound; ) {
			if (remainder[i]) {
				quotient.set(i - bound);
				remainder ^= divisor << i - bound;
			}
		}

		return { quotient, remainder };
	}

	static GFPolynomial gen_random_poly(const gf_bits& p) {
		return { gen_random_bitset<s>(), p };
	}

	static std::tuple<int, GFPolynomial> solve_sqr_eq(const GFPolynomial& u, const GFPolynomial& w) {
		// x^2 + u*x = w
		if (!u) {
			// TODO: return square of w
			return { 0,{ 0, w.p } };
		}

		if (!w) {
			return { 2,{ 0, w.p } };
		}

		auto v = w * ~(u * u);
		if (v.trace()) { // an equation has solutions if and only if its trace == 0
			return { 0,{ 0, w.p } };
		}

		return { 2, v.half_trace() * u };
	}
};
