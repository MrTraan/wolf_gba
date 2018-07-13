#include "source/types.h"

#include <stdio.h>

int main() {
	fixed a = fixed_froms32(7);
	fixed b = fixed_froms32(2);

	fixed c = fixed_fromf(7.5);

	printf("%f %f\n", fixed_tof(a), fixed_tof(b));
	printf("%x\n", fixed_div(a, b));
	printf("%f\n", fixed_tof(fixed_div(a, b)));
	printf("%x\n", fixed_mul(a, b));
	printf("%f\n", fixed_tof(fixed_mul(a, b)));
	printf("%f\n", fixed_tof(fixed_mul(a, c)));
	printf("%f\n", fixed_tof(c << 1));
	printf("%d\n", c >> 8);

	fixed d = fixed_froms32(4);
	fixed e = fixed_froms32(-7);
	printf("%f\n", fixed_tof(d + e));
	printf("%f\n", fixed_tof(fixed_mul(d, e)));
	printf("%f\n", fixed_tof(fixed_mul(e, e)));
	return (0);
}
