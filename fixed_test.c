#include "source/types.h"

#include <stdio.h>
#include <assert.h>

void test_operations();
void test_floor();
void test_round();
void test_conversions();
void test_decimal_part();

int main() {
	test_conversions();
	test_operations();
	test_decimal_part();
	test_floor();
	test_round();
	
	return (0);
}

#define FLOAT_CMP_ERROR 0.005
static inline int float_almost_equals(float a, float b) {
	if (a + FLOAT_CMP_ERROR > b && a - FLOAT_CMP_ERROR < b)
		return 1;
	return 0;
}

static struct {
	fixed lha;
	fixed rha;
	enum {
		ADD,
		SUB,
		MUL,
		DIV
	} op;
	fixed result;
} test_operations_data[] = {
	{ fixed_froms32(1), fixed_froms32(2), ADD, fixed_froms32(3) },
	{ fixed_froms32(137), fixed_froms32(14), SUB, fixed_froms32(123) },
	{ fixed_froms32(14), fixed_froms32(3), MUL, fixed_froms32(42) },
	{ fixed_froms32(60), fixed_froms32(3), DIV, fixed_froms32(20) },

	{ fixed_fromf(2.5), fixed_froms32(3), MUL, fixed_fromf(7.5) },
	{ fixed_fromf(-3.5), fixed_froms32(3), MUL, fixed_fromf(-10.5) },
	{ fixed_froms32(5), fixed_froms32(2), DIV, fixed_fromf(2.5) },
	{ fixed_fromf(-5), fixed_fromf(1.5), DIV, fixed_fromf(-3.33) },
};

void test_operations() {
	int count_test = sizeof(test_operations_data) / sizeof(*test_operations_data);

	for (int i = 0; i < count_test; i++) {
		switch(test_operations_data[i].op) {
			case ADD:
				assert(test_operations_data[i].lha + test_operations_data[i].rha == test_operations_data[i].result);
				break;
			case SUB:
				assert(test_operations_data[i].lha - test_operations_data[i].rha == test_operations_data[i].result);
				break;
			case MUL:
				assert(fixed_mul(test_operations_data[i].lha, test_operations_data[i].rha) == test_operations_data[i].result);
				break;
			case DIV:
				assert(float_almost_equals(
					fixed_tof(fixed_div(test_operations_data[i].lha, test_operations_data[i].rha)),
					fixed_tof(test_operations_data[i].result))
				);
				break;
		}
	}
}

void test_conversions() {
	for (int i = -(1 << FIX_SHIFT); i < 1 << FIX_SHIFT; i++) {
		assert(fixed_tos32(fixed_froms32(i)) == i);
	}
	
	for (float i = -(1 << FIX_SHIFT); i < 1 << FIX_SHIFT; i += 0.3) {
		assert(float_almost_equals(fixed_tof(fixed_fromf(i)), i));
	}

}

void test_overflow() {
}

void test_decimal_part() {
	fixed a = fixed_fromf(7.652);
	fixed b = fixed_fromf(193.652);
	assert(fixed_decimal_part(a) == fixed_decimal_part(b));
	
	a = fixed_fromf(89.001);
	b = fixed_fromf(-12.001);
	assert(fixed_decimal_part(a) == fixed_decimal_part(b));
}

void test_floor() {
	fixed base = fixed_fromf(7.5);
	fixed expected = fixed_froms32(7);
	assert(fixed_floor(base) == expected);
	
	base = fixed_fromf(7);
	assert(fixed_floor(base) == expected);

	base = fixed_fromf(7.52);
	assert(fixed_floor(base) == expected);

	base = fixed_fromf(7.48);
	assert(fixed_floor(base) == expected);

	base = fixed_fromf(7.48);
	assert(fixed_floor(base) == expected);

	base = fixed_fromf(7.001);
	assert(fixed_floor(base) == expected);
	
	base = fixed_fromf(7.99);
	assert(fixed_floor(base) == expected);
}

void test_round() {
	fixed base = fixed_fromf(7.5);
	fixed expected = fixed_froms32(7);
	assert(fixed_round(base) == expected);

	base = fixed_fromf(7.52);
	expected = fixed_froms32(8);
	assert(fixed_round(base) == expected);

	base = fixed_fromf(7.48);
	expected = fixed_froms32(7);
	assert(fixed_round(base) == expected);
	
	base = fixed_fromf(112.001);
	expected = fixed_froms32(112);
	assert(fixed_round(base) == expected);
	
	base = fixed_fromf(1024);
	expected = fixed_froms32(1024);
	assert(fixed_round(base) == expected);
}
