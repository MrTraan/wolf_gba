#include "types.h"
#include "video.h"
#include "player.h"
#include <math.h>
u16 __key_curr=0, __key_prev=0;
#include "input.h"
#include <string.h>

#define M4_WIDTH    240     // Width in mode 4
#define M4_HEIGHT   160     // Height in mode 4

u16* vid_page = (u16*)((u32)VID_MEM ^ VRAM_PAGE_SIZE);

inline void m4_plot(int x, int y, u8 clrid)
{
	u16 *dst= vid_page + (y * M4_WIDTH + x) / 2;  // Division by 2 due to u8/u16 pointer mismatch!
	if(x&1)
		*dst= (*dst& 0xFF) | (clrid<<8);    // odd pixel
	else
		*dst= (*dst&~0xFF) |  clrid;        // even pixel
}

inline void m4_plot2(int x, int y, u8 clr1, u8 clr2)
{
	u16 *dst= vid_page + (y*M4_WIDTH+x)/2;  // Division by 2 due to u8/u16 pointer mismatch!
	*dst = (clr2<<8) | clr1;
}

void vid_flip()
{
	// toggle the write_buffer's page
	vid_page = (u16*)((u32)vid_page ^ VRAM_PAGE_SIZE);
	REG_DISPCNT ^= DCNT_PAGE;            // update control register
}

int world_map[24][24] = {
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
	{1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

Step setup_step(Player* player, V2 ray_dir, V2_u32 map, V2 delta_dist)
{
	//TODO: optimize here with functions to get decimal part of fixed
	Step step = {};
	if (ray_dir.x < 0)
	{
		step.step_x = -1;
		step.side_dist.x = fixed_mul(player->pos.x - map.x, delta_dist.x);
	}
	else
	{
		step.step_x = 1;
		step.side_dist.x = fixed_mul(map.x - player->pos.x + fixed_froms32(1), delta_dist.x);
	}
	if (ray_dir.y < 0)
	{
		step.step_y = -1;
		step.side_dist.y = fixed_mul(player->pos.y - map.y, delta_dist.y);
	}
	else
	{
		step.step_y = 1;
		step.side_dist.y = fixed_mul(map.y - player->pos.y + fixed_froms32(1), delta_dist.y);
	}

	return step;
}

#ifndef ABS
# define ABS(x) (x >= 0 ? x : -x)
#endif

void calc_line_height(Player* player, fixed camera, V2_u32* out_draw, u8* color)
{
	V2 ray_dir = {
		.x = player->dir.x + fixed_mul(player->plane.x, camera),
		.y = player->dir.y + fixed_mul(player->plane.y, camera),
	};
	//TODO: Optimize this
	V2 delta_dist = {
		.x = ABS(fixed_div(fixed_froms32(1), ray_dir.x)),
		.y = ABS(fixed_div(fixed_froms32(1), ray_dir.y)),
	};
	V2_u32 map = {
		.x = (player->pos.x >> 8) << 8,
		.y = (player->pos.y >> 8) << 8,
	};
	Step step = setup_step(player, ray_dir, map, delta_dist);

	u8 side = 0;

	u32 cursor_x = (player->pos.x >> 8);
	u32 cursor_y = (player->pos.y >> 8);
	while (1)
	{
		if (step.side_dist.x < step.side_dist.y)
		{
			step.side_dist.x += delta_dist.x;
			cursor_x += step.step_x;
			side = 0;
		}
		else
		{
			step.side_dist.y += delta_dist.y;
			cursor_y += step.step_y;
			side = 1;
		}
		if (world_map[cursor_x][cursor_y] > 0)
			break;
	}

	fixed dist;
	if (side == 0)
		dist = fixed_div(fixed_froms32(cursor_x) - player->pos.x + ((fixed_froms32(1) - step.step_x) >> 1), ray_dir.x);
	else
		dist = fixed_div(fixed_froms32(cursor_y) - player->pos.y + ((fixed_froms32(1) - step.step_y) >> 1), ray_dir.y);

	s32 wallID = world_map[cursor_x][cursor_y];

	*color = wallID + 1;
	if (side)
		*color += 4;

	s32 line_height = (fixed_div(fixed_froms32(M4_HEIGHT), dist)) >> 9; //shift by 9 instead of 8 because we divide by two at the same time

	s32 draw_start = -line_height + M4_HEIGHT / 2;
	if (draw_start < 0)
		draw_start = 0;
	s32 draw_end = line_height + M4_HEIGHT / 2;
	if (draw_end > M4_HEIGHT)
		draw_end = M4_HEIGHT - 1;
	out_draw->x = draw_start;
	out_draw->y = draw_end;
}

#define MIN(a, b)(a > b ? b : a)
#define MAX(a, b)(a < b ? b : a)
#define SCREN_FIXW (fixed_froms32(M4_WIDTH - 1))

static fixed camera_angles_cache[M4_WIDTH] = {};

void draw_map(Player *player)
{
	for (u32 x = 0; x < M4_WIDTH; x += 2)
	{

		fixed camera_1 = camera_angles_cache[x];
		fixed camera_2 = camera_angles_cache[x + 1];

		V2_u32 draw_bounds_1;
		V2_u32 draw_bounds_2;
		u8 color1, color2;

		calc_line_height(player, camera_1, &draw_bounds_1, &color1);
		calc_line_height(player, camera_2, &draw_bounds_2, &color2);

		/* draw_bounds_1.x = 10; */
		/* draw_bounds_1.y = 120; */
		/* draw_bounds_2.x = 10; */
		/* draw_bounds_2.y = 120; */
		/* color1 = 3; */
		/* color2 = 3; */

		for (u32 y = MIN(draw_bounds_1.x, draw_bounds_2.x);
				y <= MAX(draw_bounds_1.x, draw_bounds_2.y);
				y++)
		{
			if (y >= draw_bounds_1.x && y <= draw_bounds_1.y &&
					y >= draw_bounds_2.x && y <= draw_bounds_2.y)
				m4_plot2(x, y, color1, color2);
			else if (y >= draw_bounds_1.x && y <= draw_bounds_1.y)
				m4_plot(x, y, color1);
			else if (y >= draw_bounds_2.x && y <= draw_bounds_2.y)
				m4_plot(x + 1, y, color2);
		}
	}
}

void draw_bg() {
	memset(vid_page, 10 | (10 << 8) | (10 << 16) | (10 << 24), VRAM_PAGE_SIZE);
}


void move_player(Player* player)
{
	float rotSpeed = 0.3f;
	fixed move_speed = fixed_fromf(0.3f);

	if (key_is_down(KEY_LEFT))
	{
		//both camera direction and camera plane must be rotated
		//@TODO: Use only fixed here
		float dirx = fixed_tof(player->dir.x);
		float diry = fixed_tof(player->dir.y);
		player->dir.x = fixed_fromf(dirx * cos(rotSpeed) - diry * sin(rotSpeed));
		player->dir.y = fixed_fromf(dirx * sin(rotSpeed) + diry * cos(rotSpeed));

		float planex = fixed_tof(player->plane.x);
		float planey = fixed_tof(player->plane.y);
		player->plane.x = fixed_fromf(planex * cos(rotSpeed) - planey * sin(rotSpeed));
		player->plane.y = fixed_fromf(planex * sin(rotSpeed) + planey * cos(rotSpeed));
	}
	if (key_is_down(KEY_RIGHT))
	{
		//both camera direction and camera plane must be rotated
		//@TODO: Use only fixed here
		float dirx = fixed_tof(player->dir.x);
		float diry = fixed_tof(player->dir.y);
		player->dir.x = fixed_fromf(dirx * cos(-rotSpeed) - diry * sin(-rotSpeed));
		player->dir.y = fixed_fromf(dirx * sin(-rotSpeed) + diry * cos(-rotSpeed));

		float planex = fixed_tof(player->plane.x);
		float planey = fixed_tof(player->plane.y);
		player->plane.x = fixed_fromf(planex * cos(-rotSpeed) - planey * sin(-rotSpeed));
		player->plane.y = fixed_fromf(planex * sin(-rotSpeed) + planey * cos(-rotSpeed));
	}
	if (key_is_down(KEY_UP))
	{
		fixed new_x = player->pos.x + fixed_mul(player->dir.x, move_speed);
		fixed new_y = player->pos.y + fixed_mul(player->dir.y, move_speed);
		if (world_map[new_x >> 8][player->pos.y >> 8] == 0)
			player->pos.x = new_x;
		if (world_map[player->pos.x >> 8][new_y >> 8] == 0)
			player->pos.y = new_y;
	}
	if (key_is_down(KEY_DOWN))
	{
		fixed new_x = player->pos.x - fixed_mul(player->dir.x, move_speed);
		fixed new_y = player->pos.y - fixed_mul(player->dir.y, move_speed);
		if (world_map[new_x >> 8][player->pos.y >> 8] == 0)
			player->pos.x = new_x;
		if (world_map[player->pos.x >> 8][new_y >> 8] == 0)
			player->pos.y = new_y;
	}
}

int main(void)
{
	setVideoMode(DCNT_MODE4 | DCNT_BG2);

	for (u32 x = 0; x < M4_WIDTH; x++)
		camera_angles_cache[x] = fixed_div(fixed_froms32(x << 1), SCREN_FIXW) - fixed_froms32(1);

	PAL_BG_MEM[0] = RGB15(31, 31, 31);
	PAL_BG_MEM[1] = RGB15(0, 0, 0);
	PAL_BG_MEM[2] = RGB15(0, 0, 31);
	PAL_BG_MEM[3] = RGB15(0, 31, 0);
	PAL_BG_MEM[4] = RGB15(31, 0, 0);
	PAL_BG_MEM[5] = RGB15(0, 20, 20);
	PAL_BG_MEM[6] = RGB15(0, 0, 20);
	PAL_BG_MEM[7] = RGB15(0, 20, 0);
	PAL_BG_MEM[8] = RGB15(20, 0, 0);
	PAL_BG_MEM[9] = RGB15(0, 10, 10);
	PAL_BG_MEM[10] = RGB15(10, 10, 10);

	Player player = {};
	player.pos.x = fixed_froms32(3);
	player.pos.y = fixed_froms32(3);
	player.dir.x = fixed_froms32(1);
	player.dir.y = fixed_froms32(0);
	player.plane.x = fixed_froms32(0);
	player.plane.y = fixed_fromf(-0.66f);

	while (1) {
		vid_vsync();
		vid_flip();
		draw_bg();
		draw_map(&player);
		key_poll();
		move_player(&player);
	}
	return (0);
}
