#include <math.h>
#include <string.h>
#include "types.h"
#include "video.h"
#include "player.h"
u16 __key_curr=0, __key_prev=0;
#include "input.h"

#define M4_WIDTH    240     // Width in mode 4
#define M4_HEIGHT   160     // Height in mode 4

#define BG_COLOR 10

/**
 * vid_page points to the currently used frame buffer
 * it should only be modified using vid_flip
 */
u16* vid_page = (u16*)((u32)VID_MEM ^ VRAM_PAGE_SIZE);

/**
 * vid_flip switch the frame buffer used
 */
void vid_flip()
{
	vid_page = (u16*)((u32)vid_page ^ VRAM_PAGE_SIZE);
	REG_DISPCNT ^= DCNT_PAGE;
}


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

#ifndef ABS
# define ABS(x) (x >= 0 ? x : -x)
#endif

/**
 * calc_line_height performs the DDA algorithm.
 * out_draw will be filled with the bounds of the vertical columns to draw,
 * and color will be filled with the palette index to use when drawing
 */
void calc_line_height(Player* player, fixed camera, V2_u32* out_draw, u8* color)
{
	V2 ray_dir = {
		.x = player->dir.x + fixed_mul(player->plane.x, camera),
		.y = player->dir.y + fixed_mul(player->plane.y, camera),
	};
	V2 delta_dist = {
		.x = ABS(fixed_div(fixed_froms32(1), ray_dir.x)),
		.y = ABS(fixed_div(fixed_froms32(1), ray_dir.y)),
	};

	Step step = {};
	if (ray_dir.x < 0)
	{
		step.step_x = -1;
		step.side_dist.x = fixed_mul(fixed_decimal_part(player->pos.x), delta_dist.x);
	}
	else
	{
		step.step_x = 1;
		step.side_dist.x = fixed_mul(FIXED_ONE - fixed_decimal_part(player->pos.x), delta_dist.x);
	}
	if (ray_dir.y < 0)
	{
		step.step_y = -1;
		step.side_dist.y = fixed_mul(fixed_decimal_part(player->pos.y), delta_dist.y);
	}
	else
	{
		step.step_y = 1;
		step.side_dist.y = fixed_mul(FIXED_ONE - fixed_decimal_part(player->pos.y), delta_dist.y);
	}

	u8 side = 0;

	u32 cursor_x = fixed_tos32(player->pos.x);
	u32 cursor_y = fixed_tos32(player->pos.y);
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
		dist = fixed_div(fixed_froms32(cursor_x) - player->pos.x + ((FIXED_ONE - fixed_froms32(step.step_x)) >> 1), ray_dir.x);
	else
		dist = fixed_div(fixed_froms32(cursor_y) - player->pos.y + ((FIXED_ONE - fixed_froms32(step.step_y)) >> 1), ray_dir.y);

	s32 wallID = world_map[cursor_x][cursor_y];

	*color = wallID + 1;
	if (side)
		*color += 4;

	s32 line_height = fixed_tos32(fixed_div(fixed_froms32(M4_HEIGHT), dist) >> 1);

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
#define SCREEN_FIXW (fixed_froms32(M4_WIDTH))

/**
 * Camera angles won't change from frame to frame, so we might as well compute them only once
 * and cache the result
 */
static fixed camera_angles_cache[M4_WIDTH] = {};

/**
 * Since the video bus only support 16 bits write operations,
 * while our video mode uses only 8 bits per pixel,
 * it's much more efficient to write pixels 2 by 2.
 * Sadly, contiguous pixels in memory are on the same line but different column,
 * while we draw from a vertical line. This forces us to compute 2 contiguous columns,
 * and then draw them both at the same time
 */
void draw_map(Player *player)
{
	for (u32 x = 0; x < M4_WIDTH; x += 2)
	{

		fixed camera_1 = camera_angles_cache[x];
		fixed camera_2 = camera_angles_cache[x + 1];

		V2_u32 draw_bounds_1, draw_bounds_2;
		u8 color1, color2;

		calc_line_height(player, camera_1, &draw_bounds_1, &color1);
		calc_line_height(player, camera_2, &draw_bounds_2, &color2);

		for (u32 y = MIN(draw_bounds_1.x, draw_bounds_2.x);
				y <= MAX(draw_bounds_1.y, draw_bounds_2.y);
				y++)
		{
			if (y >= draw_bounds_1.x && y <= draw_bounds_1.y &&
					y >= draw_bounds_2.x && y <= draw_bounds_2.y)
				m4_plot2(x, y, color1, color2);
			else if (y >= draw_bounds_1.x && y <= draw_bounds_1.y)
				m4_plot2(x, y, color1, BG_COLOR);
			else
				m4_plot2(x, y, BG_COLOR, color2);
		}
	}
}

/**
 * draw_bg writes grey pixels to every bit of the video memory
 * it is quite fast thanks to memset optimizations for arm processors,
 * but it may be faster to draw a sprite using another video mode
 */
void draw_bg() {
	memset(vid_page, BG_COLOR | (BG_COLOR << 8) | (BG_COLOR << 16) | (BG_COLOR << 24), VRAM_PAGE_SIZE);
}


void move_player(Player* player)
{
	fixed rotSpeed = fixed_fromf(0.15f);
	fixed cosRotSpeed = fixed_fromf(cos(rotSpeed));
	fixed sinRotSpeed = fixed_fromf(sin(rotSpeed));
	fixed cosInvRotSpeed = fixed_fromf(cos(-rotSpeed));
	fixed sinInvRotSpeed = fixed_fromf(sin(-rotSpeed));
	fixed move_speed = fixed_fromf(0.3f);

	if (key_is_down(KEY_LEFT))
	{
		//both camera direction and camera plane must be rotated
		fixed dirx = player->dir.x;
		fixed diry = player->dir.y;
		player->dir.x = fixed_mul(dirx, cosRotSpeed) - fixed_mul(diry, sinRotSpeed);
		player->dir.y = fixed_mul(dirx, sinRotSpeed) + fixed_mul(diry, cosRotSpeed);

		fixed planex = player->plane.x;
		fixed planey = player->plane.y;
		player->plane.x = fixed_mul(planex, cosRotSpeed) - fixed_mul(planey, sinRotSpeed);
		player->plane.y = fixed_mul(planex, sinRotSpeed) + fixed_mul(planey, cosRotSpeed);
	}
	if (key_is_down(KEY_RIGHT))
	{
		//both camera direction and camera plane must be rotated
		fixed dirx = player->dir.x;
		fixed diry = player->dir.y;
		player->dir.x = fixed_mul(dirx, cosInvRotSpeed) - fixed_mul(diry, sinInvRotSpeed);
		player->dir.y = fixed_mul(dirx, sinInvRotSpeed) + fixed_mul(diry, cosInvRotSpeed);

		fixed planex = player->plane.x;
		fixed planey = player->plane.y;
		player->plane.x = fixed_mul(planex, cosInvRotSpeed) - fixed_mul(planey, sinInvRotSpeed);
		player->plane.y = fixed_mul(planex, sinInvRotSpeed) + fixed_mul(planey, cosInvRotSpeed);
	}
	if (key_is_down(KEY_UP))
	{
		fixed new_x = player->pos.x + fixed_mul(player->dir.x, move_speed);
		fixed new_y = player->pos.y + fixed_mul(player->dir.y, move_speed);
		if (world_map[fixed_tos32(new_x)][fixed_tos32(player->pos.y)] == 0)
			player->pos.x = new_x;
		if (world_map[fixed_tos32(player->pos.x)][fixed_tos32(new_y)] == 0)
			player->pos.y = new_y;
	}
	if (key_is_down(KEY_DOWN))
	{
		fixed new_x = player->pos.x - fixed_mul(player->dir.x, move_speed);
		fixed new_y = player->pos.y - fixed_mul(player->dir.y, move_speed);
		if (world_map[fixed_tos32(new_x)][fixed_tos32(player->pos.y)] == 0)
			player->pos.x = new_x;
		if (world_map[fixed_tos32(player->pos.x)][fixed_tos32(new_y)] == 0)
			player->pos.y = new_y;
	}
}

int main(void)
{
	setVideoMode(DCNT_MODE4 | DCNT_BG2);

	// Compute and cache camera angles
	for (u32 x = 0; x < M4_WIDTH; x++)
		camera_angles_cache[x] = fixed_div(fixed_froms32(2 * x), SCREEN_FIXW) - fixed_froms32(1);

	// Setup color palette
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
	PAL_BG_MEM[BG_COLOR] = RGB15(10, 10, 10);

	Player player = {};
	player.pos.x = fixed_froms32(3);
	player.pos.y = fixed_froms32(3);
	player.dir.x = fixed_froms32(1);
	player.dir.y = fixed_froms32(0);
	player.plane.x = fixed_froms32(0);
	player.plane.y = fixed_fromf(-0.66f);

	while (1)
	{
		vid_vsync();
		vid_flip();
		draw_bg();
		draw_map(&player);
		key_poll();
		move_player(&player);
	}
	return (0);
}
