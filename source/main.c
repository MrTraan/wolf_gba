#include "types.h"
#include "video.h"

#define M4_WIDTH    240     // Width in mode 4

u16 *vid_page= VID_MEM;     // Point to current frame buffer

inline void
m4_plot(int x, int y, u8 clrid)
{
    u16 *dst= &vid_page[(y*M4_WIDTH+x)/2];  // Division by 2 due to u8/u16 pointer mismatch!
    if(x&1)
        *dst= (*dst& 0xFF) | (clrid<<8);    // odd pixel
    else
        *dst= (*dst&~0xFF) |  clrid;        // even pixel
}

u16 *vid_flip()
{
    // toggle the write_buffer's page
    vid_page= (u16*)((u32)vid_page ^ VRAM_PAGE_SIZE);
    REG_DISPCNT ^= DCNT_PAGE;            // update control register
    return vid_page;
}

int
main(void)
{
	setVideoMode(DCNT_MODE4 | DCNT_BG2);

	PAL_BG_MEM[0] = RGB15(31, 31, 31);
	PAL_BG_MEM[1] = RGB15(0, 0, 31);
	PAL_BG_MEM[2] = RGB15(31, 0, 0);

	for (int x = 40; x < 80; x++)
	{
		for (int y = 40; y < 80; y++)
		{
			m4_plot(x, y, 0x1);
		}
	}
	for (int x = 80; x < 120; x++)
	{
		for (int y = 80; y < 120; y++)
		{
			m4_plot(x, y, 0x2);
		}
	}

	while(1);
	return (0);
}
