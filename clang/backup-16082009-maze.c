#define __NCURSES__ 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <curses.h>

#define	MAP_DIMENSION	64
#define TEXTURE_RESOLUTION	16

#define TILE_SIZE	64
#define VIEW_ANGLE	90.0
#define	X_RESOLUTION	(80 * 1)
#define	Y_RESOLUTION	(24 * 2)
#define TO_RAD(x)	((x) * ((1.0 / 180.0) * M_PI))
#define TO_DEG(X)	((double)(x) * (180.0 / M_PI))
#define ANGLE_STEP	(TO_RAD(VIEW_ANGLE) / (X_RESOLUTION - 1))

#if __NCURSES__
#define printf(...)
#endif

static char frame_data[Y_RESOLUTION][X_RESOLUTION];


static char texture[TEXTURE_RESOLUTION][TEXTURE_RESOLUTION] = 
{
"+--------------+",
"|X            X|",
"| X          X |",
"|  X        X  |",
"|   X      X   |",
"|    X    X    |",
"|     X  X     |",
"|      XX      |",
"|      XX      |",
"|     X  X     |",
"|    X    X    |",
"|   X      X   |",
"|  X        X  |",
"| X          X |",
"|X            X|",
"---------------+",
};

static char map[MAP_DIMENSION][MAP_DIMENSION] = 
{
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"00000000000+0+0+0+0+++++0000000000000000000000000000000000000000",
"00000000000+00000000000+0000000000000000000000000000000000000000",
"00000000000+00000000000+0000000000000000000000000000000000000000",
"00000000000+00000000000+0000000000000000000000000000000000000000",
"00000000000+++++00++++000000000000000000000000000000000000000000",
"000000000000000+00+0000+0000000000000000000000000000000000000000",
"000000000000000+00+++++00000000000000000000000000000000000000000",
"000000000000000+000000+00000000000000000000000000000000000000000",
"0000000000++++++000000+00000000000000000000000000000000000000000",
"0000000000000000000000+00000000000000000000000000000000000000000",
"0000000000++++++0++++++00000000000000000000000000000000000000000",
"000000000000000+0+0000000000000000000000000000000000000000000000",
"000000000000000+0+0000000000000000000000000000000000000000000000",
"000000000000000+0+0000000000000000000000000000000000000000000000",
"000000000000000+0+0000000000000000000000000000000000000000000000",
"000000abcde00000000000000000000000000000000000000000000000000000",
"000000m#!#f00000000000000000000000000000000000000000000000000000",
"000000lkjig00000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
"0000000000000000000000000000000000000000000000000000000000000000",
};

typedef struct { float x, y; } vector;

enum RAY_CAST_AXIS
{
	RAY_CAST_INVALID = 0,
	RAY_CAST_X,
	RAY_CAST_Y,
};

static void gen_map(void)
{
int i;
	memset(map, 0, sizeof map);
	memset(map[0], 1, MAP_DIMENSION);
	memset(map[MAP_DIMENSION - 1], 1, MAP_DIMENSION);
	for (i = 1; i < MAP_DIMENSION - 1; i++)
		map[i][0] = map[i][MAP_DIMENSION - 1] = 1;
}

static struct
{
	vector	pos;
	float	view_angle;
}
camera, new_camera;

static char last_texture_byte;

static char is_map_cell_filled(float x, float y, enum RAY_CAST_AXIS cast_dir,
		int quadrant_data)
{
int map_x, map_y;

	map_x = floor(x / TILE_SIZE);
	map_y = floor(y / TILE_SIZE);
	if (cast_dir == RAY_CAST_X)
	{
		if (quadrant_data & 2)
			map_x--;
	}
	else
	{
		if (quadrant_data & 1)
			map_y--;
	}
	if (map_x <= 0 || map_y <= 0
			|| map_x >= MAP_DIMENSION - 1 || map_y >= MAP_DIMENSION - 1)
	{
		printf("out of bounds: x == %i, y == %i ", map_x, map_y);
		return '.';
	}
	//if (map[map_x][MAP_DIMENSION - 1 - map_y] ? true : false;
	if (map[MAP_DIMENSION - 1 - map_y][map_x] != '0')
	{
		return map[MAP_DIMENSION - 1 - map_y][map_x];
		//printf("map tile at %i, %i (%c) ", map_x, 
	}
	else
	{
		return 0;
	}
}

static cast_ray(vector start, float angle, int * texture_idx)
{
float sa, ca, dist_x, dist_y, x, y;
int t, map_dx, map_dy, texture_cast_x, texture_cast_y;
int texture_x, texture_y;

	printf("cast_ray(): angle is %f %f ", (double) angle, angle * (180.0 / M_PI));
	sa = sin(angle);
	ca = cos(angle);
	t = 0;
	if (sa < 0.0)
		t |= 1;
	if (ca < 0.0)
		t |= 2;
	/* see in which quadrant the angle lies */
	switch (t)
	{
		case 0:
			/* first quadrant - 0 - pi/2 */
			map_dx = 1;
			map_dy = 1;
			break;
		case 1:
			/* fourth quadrant - 3 * pi / 2 - 2 * pi */
			map_dx = 1;
			map_dy = -1;
			break;
		case 2:
			/* second quadrant - pi / 2 - pi */
			map_dx = -1;
			map_dy = 1;
			break;
		/*case 3:*/
		default:
			/* third quadrant - pi - 3 * pi / 2 */
			map_dx = -1;
			map_dy = -1;
			break;
	}
	/* r = r0 + l * rc */
	/* cast in x direction */
	if (ca != 0.0)
	{
		x = floor(start.x / TILE_SIZE) * TILE_SIZE;
		if (map_dx == 1)
			x += TILE_SIZE;
		dist_x = (x - start.x) / ca;
		y = start.y + dist_x * sa;
		while (1)
		{
			if (texture_x = is_map_cell_filled(x, y, RAY_CAST_X, t))
				break;
			if (map_dx == 1)
				x += TILE_SIZE;
			else
				x -= TILE_SIZE;
			dist_x = (x - start.x) / ca;
			y = start.y + dist_x * sa;
		}
	}
	else
		dist_x = 2 * MAP_DIMENSION * TILE_SIZE;
	texture_cast_x = (fmod(y, TILE_SIZE) / TILE_SIZE) * TEXTURE_RESOLUTION;
	/* cast in y direction */
	if (sa != 0.0)
	{
		y = floor(start.y / TILE_SIZE) * TILE_SIZE;
		if (map_dy == 1)
			y += TILE_SIZE;
		dist_y = (y - start.y) / sa;
		x = start.x + dist_y * ca;
		while (1)
		{
			if (texture_y = is_map_cell_filled(x, y, RAY_CAST_Y, t))
				break;
			if (map_dy == 1)
				y += TILE_SIZE;
			else
				y -= TILE_SIZE;
			dist_y = (y - start.y) / sa;
			x = start.x + dist_y * ca;
		}
	}
	else
		dist_y = 2 * MAP_DIMENSION * TILE_SIZE;
	texture_cast_y = (fmod(x, TILE_SIZE) / TILE_SIZE) * TEXTURE_RESOLUTION;

	if (dist_x > dist_y)
	{
		last_texture_byte = texture_y;
		*texture_idx = texture_cast_y;
		printf("cast in y direction, distance is %f\n", (double) dist_y);
	}
	else
	{
		last_texture_byte = texture_x;
		*texture_idx = texture_cast_x;
		printf("cast in x direction, distance is %f\n", (double) dist_x);
	}
	return (dist_x > dist_y) ? dist_y : dist_x;
}

static void render_frame(void)
{
int i, j, k;
float mid_angle;
float dist, col_height;
float tstep, tidx;
int texture_idx;

	memset(frame_data, ' ', sizeof frame_data);
	mid_angle = camera.view_angle + TO_RAD(VIEW_ANGLE) * 0.5;
	for (i = 0; i < X_RESOLUTION; i++)
	{
		printf("ray: %i ", i);
		dist = cast_ray(camera.pos,
				camera.view_angle + i * ANGLE_STEP,
				&texture_idx);
		dist *= cos(mid_angle - (camera.view_angle + i * ANGLE_STEP));
		col_height = 1.0 * TILE_SIZE * Y_RESOLUTION / dist;
		j = round(col_height);
		printf("dist = %f, j == %i\n", dist, j);
		if (j & 1)
			j++;
		tstep = (float) TEXTURE_RESOLUTION / (float) (j - 1);
		if (j > Y_RESOLUTION)
			j = Y_RESOLUTION, tidx = (float) (j - Y_RESOLUTION) * tstep;
		else
			tidx = .0;
		for (k = (Y_RESOLUTION - j) >> 1; j; k++, j--)
		{
			frame_data[k][X_RESOLUTION - 1 - i]
				= texture[(int) tidx][texture_idx];
			tidx += tstep;
		}
	}
#if __NCURSES__
	erase();
#endif
	for (i = 0; i < Y_RESOLUTION; i++)
	{
		frame_data[i][X_RESOLUTION - 1] = '\x00';
		printf("%s\n", frame_data[i]);
#if __NCURSES__
		mvaddstr(i, 0, frame_data[i]);
#endif
	}

}

static void init_xcurses(void)
{
	if (!initscr())
		exit(1);
	noecho();
	keypad(stdscr, TRUE);
}

int main(void)
{
int c;
#if __NCURSES__
	init_xcurses();
#endif
	camera.pos.x = 9.0 * TILE_SIZE;
	camera.pos.y = 9.0 * TILE_SIZE;
	camera.view_angle = M_PI * 1.;
	camera.pos.x = 851.;
	camera.pos.y = 252.;
	camera.view_angle = M_PI * 0.75;
	while (1)
	{
		float mid_angle, side_angle, ca, sa;
		render_frame();
		if ((c =
#if __NCURSES__
			getch()
#else
			getchar()
#endif
			) == 'q')
			break;
		mid_angle = camera.view_angle + TO_RAD(VIEW_ANGLE) * 0.5;
		side_angle = camera.view_angle - TO_RAD(VIEW_ANGLE) * 0.5;
		sa = sin(mid_angle);
		ca = cos(mid_angle);

		new_camera = camera;
		switch (c)
		{
			case 'a': new_camera.view_angle += 4.0 * ANGLE_STEP; break;
			case 'd': new_camera.view_angle -= 4.0 * ANGLE_STEP; break;
			case 'w': new_camera.pos.x += 15 * ca; new_camera.pos.y += 15 * sa; break;
			case 's': new_camera.pos.x -= 15 * ca; new_camera.pos.y -= 15 * sa; break;
			case 'c': new_camera.pos.x += 15 * cos(side_angle); new_camera.pos.y += 15 * sin(side_angle); break;
			case 'z': new_camera.pos.x -= 15 * cos(side_angle); new_camera.pos.y -= 15 * sin(side_angle); break;
		}
		if (map[MAP_DIMENSION - 1 - (int)(new_camera.pos.y / TILE_SIZE)][(int)(new_camera.pos.x / TILE_SIZE)] == '0')
			camera = new_camera;
	}
#if __NCURSES__
	endwin();
#endif
#undef printf
	printf("x = %f, y = %f, angle = %f",
			(double)camera.pos.x,
			(double)camera.pos.y,
			fmod(camera.view_angle, 2.0 * M_PI));
	return 0;
}

