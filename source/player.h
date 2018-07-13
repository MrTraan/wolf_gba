#ifndef PLAYER_H
# define PLAYER_H

#include "types.h"

typedef struct {
	V2 pos;
	V2 dir;
	V2 plane;
} Player;

typedef struct {
	s32 step_x;
	s32 step_y;
	V2 side_dist;
} Step;

typedef struct {
	fixed dist;
	u8 side;
	int wallID;
} DDARecord;

#endif
