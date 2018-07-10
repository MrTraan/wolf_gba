#ifndef SYSTEM_H
# define SYSTEM_H

#include "types.h"

#define MEM_IO		0x04000000
#define MEM_PAL		0x05000000		// no 8bit write !!
#define MEM_VRAM	0x06000000		// no 8bit write !!

#define PAL_SIZE	0x00400
#define VRAM_SIZE	0x18000

#define M3_SIZE			0x12C00
#define M4_SIZE			0x09600
#define M5_SIZE			0x0A000
#define VRAM_PAGE_SIZE	0x0A000

#define REG_BASE	MEM_IO
#define REG_DISPCNT			*(vu32*)(REG_BASE+0x0000)	// display control


#define MEM_VRAM_BACK	(MEM_VRAM+ VRAM_PAGE_SIZE)

#endif
