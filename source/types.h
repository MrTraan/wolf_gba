#ifndef TYPES_H
# define TYPES_H

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

#define fixed_fromf(f) (fixed)((float)(f) * (1 << FIX_SHIFT))

#define fixed_froms32(d) (fixed)((s32)(d) << FIX_SHIFT)

static inline s32 fixed_decimal_part(fixed d) {
	return d & ((1 << FIX_SHIFT) - 1);
}


static inline fixed fixed_floor(fixed d) {
	return d & ~(1 << (FIX_SHIFT - 1));
}

static inline fixed fixed_round(fixed d) {
	s32 decimal_part = fixed_decimal_part(d);
	// If $FIXED_SHIFT bit is not set, floor number
	if ((decimal_part ^ (1 << (FIX_SHIFT - 1))) == decimal_part)
		return fixed_floor(d);
	return fixed_floor(d) + (1 << FIX_SHIFT);
}

static inline float fixed_tof(fixed d) {
	return (float)d / (1 << FIX_SHIFT);
}

static inline s32 fixed_tos32(fixed d) {
	return d >> FIX_SHIFT;
}

static inline fixed fixed_mul(fixed a, fixed b) {
	return (a * b) >> FIX_SHIFT;
}

static inline fixed fixed_div(fixed a, fixed b) {
	return (a << FIX_SHIFT) / b;
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
