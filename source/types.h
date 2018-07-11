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

typedef struct {
	float x;
	float y;
} V2;

typedef struct {
	u32 x;
	u32 y;
} V2_u32;


#endif
