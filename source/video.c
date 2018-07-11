#include "video.h"
#include "system.h"

void
setVideoMode(u32 mode)
{
	REG_DISPCNT = mode;
}

void vid_vsync()
{
    while(REG_VCOUNT >= 160);   // wait till VDraw
    while(REG_VCOUNT < 160);    // wait till VBlank
}
