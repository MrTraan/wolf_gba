#ifndef TYPES_H
# define TYPES_H

#include <math.h>

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;

typedef char	s8;
typedef short	s16;
typedef int		s32;

typedef volatile u8		vu8;
typedef volatile u16	vu16;
typedef volatile u32	vu32;

// Custom fixed operations, 24.8 format
typedef s32 fixed;
#define FIX_SHIFT 8

static inline float fixed_tof(fixed d) {
	return (float)d / (1 << FIX_SHIFT);
}

static inline fixed fixed_fromf(float f) {
	return (fixed)roundf(f * (1 << FIX_SHIFT));
}

static inline fixed fixed_froms32(s32 d) {
	return d << FIX_SHIFT;
}

static inline fixed fixed_mul(fixed a, fixed b) {
	/* return (a >> FIX_SHIFT) * (b >> FIX_SHIFT); */
	return (a * b) / (1 << FIX_SHIFT);
}

static inline fixed fixed_div(fixed a, fixed b) {
	return (a * (1 << FIX_SHIFT)) / b;
}

typedef struct {
	fixed x;
	fixed y;
} V2;

typedef struct {
	u32 x;
	u32 y;
} V2_u32;

#endif
