#define V10

#include "Utils.h"
#include "ECPoint.h"
#include "GFPolynomial.h"

#include <chrono>
#include <bitset>
#include <iostream>

#include <assert.h>

void test_multiplication() {
	std::bitset<4> p = 0b1011;

	assert((GFPolynomial<3>{ 0b111, p } * GFPolynomial<3>{ 0b101, p } == GFPolynomial<3>{ 0b110, p }));
	assert((GFPolynomial<3>{ 0b100, p } * GFPolynomial<3>{ 0b110, p } == GFPolynomial<3>{ 0b101, p }));
	assert((GFPolynomial<3>{ 0b100, p } * GFPolynomial<3>{ 0b0, p	} == GFPolynomial<3>{ 0b0, p }));
	
	std::cout << "test_multiplication: SUCCESS" << std::endl;
}

void test_division() {
	auto [quotient, remainder] = GFPolynomial<3>::divide(0b1011, 0b101);
	assert(quotient == 0b10 && remainder == 0b1);

	std::tie(quotient, remainder) = GFPolynomial<3>::divide(0b1111, 0b11);
	assert(quotient == 0b101 && remainder == 0b0);

	std::tie(quotient, remainder) = GFPolynomial<3>::divide(0b111, 0b111);
	assert(quotient == 0b1 && remainder == 0b0);

	std::tie(quotient, remainder) = GFPolynomial<3>::divide(0b11, 0b101);
	assert(quotient == 0b0 && remainder == 0b11);

	std::tie(quotient, remainder) = GFPolynomial<3>::divide(0b0, 0b101);
	assert(quotient == 0b0 && remainder == 0b0);

	std::cout << "test_division: SUCCESS" << std::endl;
}

void test_inversion() {
	std::bitset<9> p = 0b100011011;
	GFPolynomial<8> a = { 0b1010011, p };
	
	assert((~a == GFPolynomial<8>{ 0b011001010, p }));
	assert((~a * a == GFPolynomial<8>{ 0b1, p }));
	assert((~a * ~a == ~(a * a)));

	std::cout << "test_inversion: SUCCESS" << std::endl;
}

void test_simple_sqr_eq_solution() {
	std::bitset<4> p = 0b1011;
	GFPolynomial<3> u = { 0b101, p };
	GFPolynomial<3> w = { 0b101, p };
	auto [solutions, x] = GFPolynomial<3>::solve_sqr_eq(u, w);

	assert(solutions);
	assert(x * x + u * x == w);

	std::cout << "test_simple_sqr_eq_solution: SUCCESS" << std::endl;
}

void test_sqr_eq_solution_failure() {
	std::bitset<9> p = 0b100011011;
	GFPolynomial<8> u = { 0b1010011, p };
	GFPolynomial<8> w = { 0b110011, p };

	auto [solutions, x] = GFPolynomial<8>::solve_sqr_eq(u, w);

	std::cout << solutions << std::endl;
	std::cout << x << std::endl;
	std::cout << std::boolalpha << (x * x + u * x == w) << std::endl;
}

void test_sqr_eq_solution() {
	static const size_t m = 163;

	auto p = bitset_from_coefs<m + 1>({ 0, 3, 6, 7, 163 });
	auto a = GFPolynomial<m>{ 0b1, p };
	auto b = GFPolynomial<m>{ hex_to_bin<m + 1>("5FF6108462A2DC8210AB403925E638A19C1455D21"), p };

	while (true) {
		auto x = GFPolynomial<m>::gen_random_poly(p);
		auto [solutions, y] = GFPolynomial<m>::solve_sqr_eq(x, x * x * x + a * x * x + b);

		if (solutions) {
			assert((y));
			assert((y * y + y * x == x * x * x + a * x * x + b));
			return;
		}
	}

	std::cout << "test_sqr_eq_solution: SUCCESS" << std::endl;
}

void test_ec_point_generation() {
	ECPoint p;
	
	// y^2 + x * y = x^3 + a * x^2 + b mod f
	assert((p.y * p.y + p.y * p.x == p.x * p.x * p.x + ECPoint::a * p.x * p.x + ECPoint::b));

	std::cout << "test_ec_point_generation: SUCCESS" << std::endl;
}

void test_ec_point_addition() {
	ECPoint p1, p2;

	assert(ECPoint::zero() + ECPoint::zero() == ECPoint::zero());
	assert(p1 + ECPoint::zero() == p1);
	assert(ECPoint::zero() + p1 == p1);

	auto p = p1 + p2;
	assert((p.y * p.y + p.y * p.x == p.x * p.x * p.x + ECPoint::a * p.x * p.x + ECPoint::b));

	p = p1 + p1;
	assert((p.y * p.y + p.y * p.x == p.x * p.x * p.x + ECPoint::a * p.x * p.x + ECPoint::b));
	
	std::cout << "test_ec_point_addition: SUCCESS" << std::endl;
}

void test_ec_point_multiplication() {
	ECPoint point;
	
	assert(point * std::bitset<64>{ 0 } == ECPoint::zero());
	assert(point * std::bitset<64>{ 1 } == point);
	assert(point * std::bitset<64>{ 2 } == point + point);
	assert(point * std::bitset<64>{ 7 } == point + point + point + point + point + point + point);

	auto p = point * std::bitset<64>{ 2 };
	assert((p.y * p.y + p.y * p.x == p.x * p.x * p.x + ECPoint::a * p.x * p.x + ECPoint::b));

	std::cout << "test_ec_point_multiplication: SUCCESS" << std::endl;
}

void test_ec_point_performance() {
	// Measure EC point generation
	auto duration = measure_exec_time([&] { 
		ECPoint point;
	});

	std::cout << "EC point generation: " << duration / 1000 << "." << duration % 1000 << " ms" << std::endl;

	// Measure EC point multiplication
	ECPoint p1, p2;
	duration = measure_exec_time([&] { 
		p1 + p2;
	});

	std::cout << "EC point addition: " << duration / 1000 << "." << duration % 1000 << " ms" << std::endl;
	
	// Measure EC point multiplication
	ECPoint point;
	auto value = gen_random_bitset<ECPoint::m>();
	duration = measure_exec_time([&] { 
		point * value;
	});

	std::cout << "EC point multiplication: " << duration / 1000 << "." << duration % 1000 << " ms" << std::endl;
}

void main() {
	std::cout << "GF(2^m): m = " << ECPoint::m << "\n\n";

	test_multiplication();
	test_division();
	test_inversion();
	test_simple_sqr_eq_solution();
	test_sqr_eq_solution();
	test_ec_point_generation();
	test_ec_point_addition();
	test_ec_point_multiplication();
	
#ifndef _DEBUG
	test_ec_point_performance();
#endif

	getchar();
}