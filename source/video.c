#include "video.h"
#include "system.h"

void
setVideoMode(u32 mode)
{
	REG_DISPCNT = mode;
}
