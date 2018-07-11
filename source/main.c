#include "types.h"
#include "video.h"
#include "player.h"
#include <math.h>
	u16 __key_curr=0, __key_prev=0;
#include "input.h"

#define M4_WIDTH    240     // Width in mode 4
#define M4_HEIGHT   160     // Height in mode 4

u16 *vid_page= VID_MEM;     // Point to current frame buffer

inline void m4_plot(int x, int y, u8 clrid)
{
	u16 *dst= &vid_page[(y*M4_WIDTH+x)/2];  // Division by 2 due to u8/u16 pointer mismatch!
	if(x&1)
		*dst= (*dst& 0xFF) | (clrid<<8);    // odd pixel
	else
		*dst= (*dst&~0xFF) |  clrid;        // even pixel
}

inline void m4_plot2(int x, int y, u8 clr1, u8 clr2)
{
	u16 *dst= &vid_page[(y*M4_WIDTH+x)/2];  // Division by 2 due to u8/u16 pointer mismatch!
	*dst = (clr1<<8) | clr2;
}

u16* vid_flip(u16 *vid_page)
{
	// toggle the write_buffer's page
	vid_page = (u16*)((u32)vid_page ^ VRAM_PAGE_SIZE);
	REG_DISPCNT ^= DCNT_PAGE;            // update control register
	return vid_page;
}

int world_map[6][6] = {
	{1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0},
	{1, 0, 0, 0, 0, 0},
	{1, 1, 1, 1, 1, 1},
};

void perform_dda(V2 pos, V2_u32 map, Step step, V2 ray_dir, V2 delta_dist, DDARecord *rec)
{
	u8 side = 0;

	while (1)
	{
		if (step.side_dist.x < step.side_dist.y)
		{
			step.side_dist.x += delta_dist.x;
			map.x += step.step_x;
			side = 0;
		}
		else
		{
			step.side_dist.y += delta_dist.y;
			map.y += step.step_y;
			side = 1;
		}
		if (world_map[map.x][map.y] > 0)
			break;
	}

	float perp_wall_dist;
	if (side == 0)
		perp_wall_dist = (map.x - pos.x + (1 - step.step_x) / 2) / ray_dir.x;
	else
		perp_wall_dist = (map.y - pos.y + (1 - step.step_y) / 2) / ray_dir.y;

	rec->dist = perp_wall_dist;
	rec->side = side;
	rec->wallID = 1;
}

Step setup_step(Player* player, V2 ray_dir, V2_u32 map, V2 delta_dist)
{
	Step step = {};
	if (ray_dir.x < 0)
	{
		step.step_x = -1;
		step.side_dist.x = (player->pos.x - map.x) * delta_dist.x;
	}
	else
	{
		step.step_x = 1;
		step.side_dist.x = (map.x + 1 - player->pos.x) * delta_dist.x;
	}
	if (ray_dir.y < 0)
	{
		step.step_y = -1;
		step.side_dist.y = (player->pos.y - map.y) * delta_dist.y;
	}
	else
	{
		step.step_y = 1;
		step.side_dist.y = (map.y + 1 - player->pos.y) * delta_dist.y;
	}

	return step;
}

void draw_map(Player *player)
{
	for (u32 x = 0; x < M4_WIDTH; x += 2)
	{
		float camera_1 = 2 * x / (float)M4_WIDTH - 1;
		float camera_2 = 2 * (x + 1) / (float)M4_WIDTH - 1;

		V2 ray_dir_1 = {
			.x = player->dir.x + player->plane.x * camera_1,
			.y = player->dir.y + player->plane.y * camera_1,
		};
		V2 ray_dir_2 = {
			.x = player->dir.x + player->plane.x * camera_2,
			.y = player->dir.y + player->plane.y * camera_2,
		};

		V2 delta_dist_1 = {
			.x = fabs(1 / ray_dir_1.x),
			.y = fabs(1 / ray_dir_1.y),
		};
		V2 delta_dist_2 = {
			.x = fabs(1 / ray_dir_2.x),
			.y = fabs(1 / ray_dir_2.y),
		};

		V2_u32 map = {
			.x = (u32)player->pos.x,
			.y = (u32)player->pos.y
		};

		Step step1 = setup_step(player, ray_dir_1, map, delta_dist_1);
		Step step2 = setup_step(player, ray_dir_2, map, delta_dist_2);

		DDARecord rec1;
		DDARecord rec2;
		perform_dda(player->pos, map, step1, ray_dir_1, delta_dist_1, &rec1);
		perform_dda(player->pos, map, step2, ray_dir_2, delta_dist_2, &rec2);

		u8 color1 = 2;
		u8 color2 = 2;
		if (rec1.side)
			color1 = 3;
		if (rec2.side)
			color2 = 3;

		s32 line_height_1 = (s32)(M4_HEIGHT / rec1.dist);
		s32 line_height_2 = (s32)(M4_HEIGHT / rec2.dist);

		s32 draw_start_1 = -line_height_1 / 2 + M4_HEIGHT / 2;
		if (draw_start_1 < 0)
			draw_start_1 = 0;
		s32 draw_end_1 = line_height_1 / 2 + M4_HEIGHT / 2;
		if (draw_end_1 > M4_HEIGHT)
			draw_end_1 = M4_HEIGHT - 1;

		s32 draw_start_2 = -line_height_2 / 2 + M4_HEIGHT / 2;
		if (draw_start_2 < 0)
			draw_start_2 = 0;
		s32 draw_end_2 = line_height_2 / 2 + M4_HEIGHT / 2;
		if (draw_end_2 > M4_HEIGHT)
			draw_end_2 = M4_HEIGHT - 1;

		for (u16 y = 0; y < M4_HEIGHT; y++)
		{
			u8 c1 = 1;
			u8 c2 = 1;
			if (y > draw_start_1 && y < draw_end_1)
				c1 = color1;
			if (y > draw_start_2 && y < draw_end_2)
				c2 = color2;
			/* m4_plot2(x, y, c1, c2); */
			m4_plot(x, y, c1);
			m4_plot(x + 1, y, c2);
		}
	}
}

int main(void)
{
	setVideoMode(DCNT_MODE4 | DCNT_BG2);

	PAL_BG_MEM[0] = RGB15(31, 31, 31);
	PAL_BG_MEM[1] = RGB15(0, 0, 0);
	PAL_BG_MEM[2] = RGB15(0, 0, 31);
	PAL_BG_MEM[3] = RGB15(0, 0, 20);

	Player player = {};
	player.pos.x = 3.0f;
	player.pos.y = 3.0f;
	player.dir.x = 1.0f;
	player.dir.y = 0.0f;
	player.plane.x = 0.0f;
	player.plane.y = -0.66f;

	float rotSpeed = 0.3f;

	while (1) {
		vid_vsync();
		draw_map(&player);
		key_poll();
		if (key_is_down(KEY_LEFT))
		{
			//both camera direction and camera plane must be rotated
			float oldDirX = player.dir.x;
			player.dir.x = player.dir.x * cos(rotSpeed) - player.dir.y * sin(rotSpeed);
			player.dir.y = oldDirX * sin(rotSpeed) + player.dir.y * cos(rotSpeed);
			float oldPlaneX = player.plane.x;
			player.plane.x = player.plane.x * cos(rotSpeed) - player.plane.y * sin(rotSpeed);
			player.plane.y = oldPlaneX * sin(rotSpeed) + player.plane.y * cos(rotSpeed);
		}
		if (key_is_down(KEY_RIGHT))
		{
			//both camera direction and camera plane must be rotated
			float oldDirX = player.dir.x;
			player.dir.x = player.dir.x * cos(-rotSpeed) - player.dir.y * sin(-rotSpeed);
			player.dir.y = oldDirX * sin(-rotSpeed) + player.dir.y * cos(-rotSpeed);
			float oldPlaneX = player.plane.x;
			player.plane.x = player.plane.x * cos(-rotSpeed) - player.plane.y * sin(-rotSpeed);
			player.plane.y = oldPlaneX * sin(-rotSpeed) + player.plane.y * cos(-rotSpeed);
		}
	}
	return (0);
}
