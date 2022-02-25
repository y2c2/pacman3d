/* Pacman 3D -- a simple 3D game built with OpenGL
   Copyright(C) 2012 y2c2 

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. 
   */

#define GL_GLEXT_PROTOTYPES

#ifndef GL_BGR_EXT
#define GL_BGR_EXT 0x80E0
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

#define UNIX 1
#if defined(WINDOWS)
#include <windows.h>
#include <GL/glu.h>
/* patch for undefined reference to `glWindowPos2i' error on old builds of opengl*/
void glWindowPos4f(float x,float y,float z,float w)
{
	//  Integer versions of x and y
	int ix = (int)x;
	int iy = (int)y;
	//  Save transform attributes (Matrix Mode and Viewport)
	glPushAttrib(GL_TRANSFORM_BIT|GL_VIEWPORT_BIT);
	//  Save projection matrix and set identity
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//  Save model view matrix and set to indentity
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//  Set viewport to 2x2 pixels around (x,y)
	glViewport(ix-1,iy-1,2,2);
	//  Finally set the raster position
	glRasterPos4f(x-ix,y-iy,z,w);
	//  Reset model view matrix
	glPopMatrix();
	//  Reset projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	//  Pop transform attributes (Matrix Mode and Viewport)
	glPopAttrib();
}

/*
 *  glWindowPos wrappers
 */
void glWindowPos2s(short  x,short  y)                   {glWindowPos4f(x,y,0,1);}
void glWindowPos2i(int    x,int    y)                   {glWindowPos4f(x,y,0,1);}
void glWindowPos2f(float  x,float  y)                   {glWindowPos4f(x,y,0,1);}
void glWindowPos2d(double x,double y)                   {glWindowPos4f(x,y,0,1);}
void glWindowPos3s(short  x,short  y,short  z)          {glWindowPos4f(x,y,z,1);}
void glWindowPos3i(int    x,int    y,int    z)          {glWindowPos4f(x,y,z,1);}
void glWindowPos3f(float  x,float  y,float  z)          {glWindowPos4f(x,y,z,1);}
void glWindowPos3d(double x,double y,double z)          {glWindowPos4f(x,y,z,1);}
void glWindowPos4s(short  x,short  y,short  z,short  w) {glWindowPos4f(x,y,z,w);}
void glWindowPos4i(int    x,int    y,int    z,int    w) {glWindowPos4f(x,y,z,w);}
void glWindowPos4d(double x,double y,double z,double w) {glWindowPos4f(x,y,z,w);}
void glWindowPos2sv(const short  v[2])                  {glWindowPos4f(v[0],v[1],  0 ,  1 );}
void glWindowPos2iv(const int    v[2])                  {glWindowPos4f(v[0],v[1],  0 ,  1 );}
void glWindowPos2fv(const float  v[2])                  {glWindowPos4f(v[0],v[1],  0 ,  1 );}
void glWindowPos2dv(const double v[2])                  {glWindowPos4f(v[0],v[1],  0 ,  1 );}
void glWindowPos3sv(const short  v[3])                  {glWindowPos4f(v[0],v[1],v[2],  1 );}
void glWindowPos3iv(const int    v[3])                  {glWindowPos4f(v[0],v[1],v[2],  1 );}
void glWindowPos3fv(const float  v[3])                  {glWindowPos4f(v[0],v[1],v[2],  1 );}
void glWindowPos3dv(const double v[3])                  {glWindowPos4f(v[0],v[1],v[2],  1 );}
void glWindowPos4sv(const short  v[4])                  {glWindowPos4f(v[0],v[1],v[2],v[3]);}
void glWindowPos4iv(const int    v[4])                  {glWindowPos4f(v[0],v[1],v[2],v[3]);}
void glWindowPos4fv(const float  v[4])                  {glWindowPos4f(v[0],v[1],v[2],v[3]);}
void glWindowPos4dv(const double v[4])                  {glWindowPos4f(v[0],v[1],v[2],v[3]);}
#endif

#include <GL/glut.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <memory.h>

/* game state */
#define GAME_STATE_TITLE 0
#define GAME_STATE_RUNNING 1
#define GAME_STATE_WIN 2
#define GAME_STATE_LOSE 3
int game_state = GAME_STATE_TITLE;

/* game pace control */
#if defined (UNIX)  
#include <sys/time.h>  
#elif defined (WINDOWS)  
#include <windows.h>  
#else  
#error "You need to define Operation System first"
#endif  

/*#define PACE_DELTA (500000 / 1)*/
/*#define PACE_SLICE_RATE (16)*/
#define PACE_DELTA (500000 / 4)
#define PACE_SLICE_RATE (8)
#define PACE_SLICE_DELTA (PACE_DELTA/PACE_SLICE_RATE)

int pace = 0;

/* constants */
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300

/* bitmap file formap supporting */
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef signed int s32;
typedef signed short s16;
typedef signed char s8;

struct bmp_file_magic
{
	u8 magic[2];
};

struct bmp_file_header
{
	u32 file_size;
	u16 reserved1;
	u16 reserved2;
	u32 data_offset;
};

struct bmp_info_header
{
	u32 size; /* bmp info structure */
	u32 width;
	u32 height;
	u16 planes;
	u16 bit_count;
	u32 compression;
	u32 size_image;
	u32 x_pixels_per_meter;
	u32 y_pixels_per_meter;
	u32 color_used;
	u32 color_important;
};

struct color_24bit
{
	u8 color[3];
};

inline static int read_s8(s8 *value, FILE *fp)
{
	if (!(fread(value, sizeof(s8), 1, fp) > 0)) return -1;
	return 0;
}

inline static int write_u32(u32 value, FILE *fp)
{
	if (fwrite(&value, sizeof(u32), 1, fp) != 1) return -1;
	return 0;
}

inline static int read_u32(u32 *value, FILE *fp)
{
	if (!(fread(value, sizeof(u32), 1, fp) > 0)) return -1;
	return 0;
}

struct bmp_image
{
	unsigned int width;
	unsigned int height;
	struct color_24bit *pixels;
};

int bmp_load(struct bmp_image **bmp, char *filename_src)
{
	int ret = 0;
	FILE *fp_src = NULL;
	struct bmp_image *new_bmp = NULL;
	*bmp = NULL;
	fp_src = fopen(filename_src, "rb");
	if (fp_src == NULL) goto fail;
	unsigned int file_len;
	fseek(fp_src, 0, SEEK_END);
	file_len = ftell(fp_src);
	fseek(fp_src, 0, SEEK_SET);
	struct bmp_file_magic file_magic;
	struct bmp_file_header file_header;
	struct bmp_info_header info_header;
	if (!(fread(&file_magic, sizeof(struct bmp_file_magic), 1, fp_src) > 0)) goto fail;
	if (!(file_magic.magic[0] == 0x42 && file_magic.magic[1] == 0x4D)) goto fail;
	if (!(fread(&file_header, sizeof(struct bmp_file_header), 1, fp_src) > 0)) goto fail;
	if (!(fread(&info_header, sizeof(struct bmp_info_header), 1, fp_src) > 0)) goto fail;
	/* currently support 24bit non-compress BMP */
	if (!((info_header.compression == 0) && (info_header.bit_count == 24)))
	{
		/* this is all support at this time */
		goto fail;
	}
	/* jump to data area */
	fseek(fp_src, file_header.data_offset, SEEK_SET);
	new_bmp = (struct bmp_image *)malloc(sizeof(struct bmp_image));
	if (new_bmp == NULL) goto fail;
	new_bmp->width = info_header.width;
	new_bmp->height = info_header.height;
	new_bmp->pixels = NULL;
	new_bmp->pixels = (struct color_24bit *)malloc(sizeof(struct color_24bit) * info_header.width * info_header.height);
	if (new_bmp->pixels == NULL) goto fail;
	if (fread(new_bmp->pixels, sizeof(struct color_24bit), info_header.width * info_header.height, fp_src) < info_header.width * info_header.height)
	{
		goto fail;
	}
	ret = 0;
	*bmp = new_bmp;
	goto done;
fail:
	ret = -1;
	if (fp_src) fclose(fp_src);
	if (new_bmp)
	{
		if (new_bmp->pixels) free(new_bmp->pixels);
		free(new_bmp);
	}
done:
	return ret;
}

int bmp_draw(struct bmp_image *bmp, int p_x, int p_y)
{
	glLoadIdentity();  
	glWindowPos2i((glutGet(GLUT_WINDOW_WIDTH)- bmp->width) / 2, (glutGet(GLUT_WINDOW_HEIGHT) - bmp->height) / 2);
	glDrawPixels(bmp->width, bmp->height, GL_BGR_EXT,GL_UNSIGNED_BYTE,bmp->pixels);
	/*glFlush();*/
	return 0;
}

int bmp_unload(struct bmp_image *bmp)
{
	if (bmp)
	{
		if (bmp->pixels) free(bmp->pixels);
		free(bmp);
	}
	return 0;
}

struct bmp_image *bmp_you_win = NULL, *bmp_you_lose = NULL, *bmp_title = NULL;

int load_resource()
{
	int ret;
	ret = bmp_load(&bmp_title, "image_title.bmp");
	ret = bmp_load(&bmp_you_win, "image_you_win.bmp");
	ret = bmp_load(&bmp_you_lose, "image_you_lose.bmp");
	if (ret) printf("Error occur while loading images\n");
	return 0;
};

/* global variables */
#define DEPTH_INIT 10
int depth = DEPTH_INIT;

/* key state */
#define KEY_STATE_DOWN 0
#define KEY_STATE_UP 1
/*int key_state = KEY_STATE_UP;*/
int key_state_up = KEY_STATE_UP;
int key_state_down = KEY_STATE_UP;
int key_state_left = KEY_STATE_UP;
int key_state_right = KEY_STATE_UP;
int key_state_enter = KEY_STATE_UP;
#define KEY_DOWN (key_state_up == KEY_STATE_DOWN || key_state_down == KEY_STATE_DOWN || key_state_left == KEY_STATE_DOWN || key_state_right == KEY_STATE_DOWN)
#define KEY_UP (!(KEY_DOWN))

int current_key;

/* environmental light */
static float diffuseLight[] = { 0.8f, 0.8f, 0.8f, 0.5f };	 
static float specularLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };	 
static float lightPosition[] = { 0.0f, 0.0f, -40.0f, 1.0f };

static float lightPositionR[] = { 3.0f,-3.0f, 0.0f, 1.0f };
static float lightPositionG[] = { 3.0f, 0.0f, 0.0f, 1.0f };
static float lightPositionB[] = { -3.0f, 0.0f, 0.0f, 1.0f };

static float diffuseLightR[] = { 1.0f, 0.0f, 0.0f, 1.0f };
static float diffuseLightG[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static float diffuseLightB[] = { 0.0f, 0.0f, 1.0f, 1.0f };

static float specularLightR[] = { 1.0f, 0.0f, 0.0f, 1.0f };
static float specularLightG[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static float specularLightB[] = { 0.0f, 0.0f, 1.0f, 1.0f };

/* map */

/* 0: Road */
/* 1: Wall */
/* 2: Bean */
#define MAP_ROAD 0
#define MAP_WALL 1
#define MAP_BEAN 2
unsigned char map_data[] = 
{
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
	1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,2,1,
	1,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,1,
	1,1,1,2,1,2,1,2,1,1,1,1,1,2,1,2,1,2,1,1,1,
	1,2,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,2,1,
	1,2,1,1,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,2,1,
	1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};
#define MAP_WIDTH 21
#define MAP_HEIGHT 9
unsigned char map_data_working[MAP_WIDTH * MAP_HEIGHT];

#define MAP_LOCATE(x,y) ((y)*MAP_WIDTH+(x))

/* beans counter */
int beans_total;
int beans_eat;

/* directions for objects */
enum {
	DIRECTION_NONE = 0, /* no about to go any way */
	DIRECTION_UP = 1,
	DIRECTION_DOWN = 2,
	DIRECTION_LEFT = 3,
	DIRECTION_RIGHT = 4,
};

/* location of objects */
struct character
{
	int x;
	int y;
	int direction;
	int next_direction;
};

struct character characters[5];

/* Draw abstract 3D objects */

void draw_box(int x, int y, float r, float g, float b, int direction)
{ 

	float x_pace_fix = 0.0, y_pace_fix = 0.0;
	switch (direction)
	{
		case DIRECTION_UP:
			y_pace_fix = 0 - (float)pace / PACE_SLICE_RATE + 1;
			break;
		case DIRECTION_DOWN:
			y_pace_fix = +(float)pace / PACE_SLICE_RATE - 1;
			break;
		case DIRECTION_LEFT:
			x_pace_fix = ((float)pace / PACE_SLICE_RATE) - 1;
			break;
		case DIRECTION_RIGHT:
			x_pace_fix = 1 - ((float)pace / PACE_SLICE_RATE);
			break;
		default:
			break;
	}

	glLoadIdentity();  
	glPushMatrix();
	glTranslatef (x + x_pace_fix, y + y_pace_fix, -depth);
	glColor3f(r, g, b);

	glBegin(GL_QUADS);
	glNormal3f(0.0f,1.0f,0.0f);
	glVertex3f(0.5f,0.5f,0.5f);
	glVertex3f(0.5f,0.5f,-0.5f);
	glVertex3f(-0.5f,0.5f,-0.5f);
	glVertex3f(-0.5f,0.5f,0.5f);
	glEnd();
	glBegin(GL_QUADS);
	glNormal3f(0.0f,-1.0f,0.0f);
	glVertex3f(0.5f,-0.5f,0.5f);
	glVertex3f(-0.5f,-0.5f,0.5f);
	glVertex3f(-0.5f,-0.5f,-0.5f);
	glVertex3f(0.5f,-0.5f,-0.5f);
	glEnd();
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,1.0f);
	glVertex3f(0.5f,0.5f,0.5f);
	glVertex3f(-0.5f,0.5f,0.5f);
	glVertex3f(-0.5f,-0.5f,0.5f);
	glVertex3f(0.5f,-0.5f,0.5f);
	glEnd();
	glBegin(GL_QUADS);
	glNormal3f(0.0f,0.0f,-1.0f);
	glVertex3f(0.5f,0.5f,-0.5f);
	glVertex3f(0.5f,-0.5f,-0.5f);
	glVertex3f(-0.5f,-0.5f,-0.5f);
	glVertex3f(-0.5f,0.5f,-0.5f);
	glEnd();
	glBegin(GL_QUADS);
	glNormal3f(-1.0f,0.0f,0.0f);
	glVertex3f(-0.5f,0.5f,0.5f);
	glVertex3f(-0.5f,0.5f,-0.5f);
	glVertex3f(-0.5f,-0.5f,-0.5f);
	glVertex3f(-0.5f,-0.5f,0.5f);
	glEnd();
	glBegin(GL_QUADS);    //右面
	glNormal3f(1.0f,0.0f,0.0f);
	glVertex3f(0.5f,0.5f,0.5f);
	glVertex3f(0.5f,-0.5f,0.5f);
	glVertex3f(0.5f,-0.5f,-0.5f);
	glVertex3f(0.5f,0.5f,-0.5f);
	glEnd();

	glPopMatrix();
}


void draw_shpere(int x, int y, float r, float g, float b, float size, float x_pace_fix, float y_pace_fix) 
{
	float fix = 0.5;
	glLoadIdentity();  
	glTranslatef ((float)x + fix + x_pace_fix - 0.5, (float)y + fix + y_pace_fix - 0.5, -depth);
	glColor3f(r, g, b);
	glutSolidSphere(size, 16, 16); 
}


/* Draw game elements */

void draw_wall_block(int x, int y, int direction)
{
	draw_box(x, y, 0.078, 0.211, 1.0, direction);
}

void draw_main_character()
{
	draw_shpere(0, 0, 1.0, 1.0, 0.0, 0.5, 0, 0);
}

void draw_bean(int x, int y, int direction)
{
	float x_pace_fix = 0.0, y_pace_fix = 0.0;
	switch (characters[0].direction)
	{
		case DIRECTION_UP:
			y_pace_fix = 0 - (float)pace / PACE_SLICE_RATE + 1;
			break;
		case DIRECTION_DOWN:
			y_pace_fix = +(float)pace / PACE_SLICE_RATE - 1;
			break;
		case DIRECTION_LEFT:
			x_pace_fix = ((float)pace / PACE_SLICE_RATE) - 1;
			break;
		case DIRECTION_RIGHT:
			x_pace_fix = 1 - ((float)pace / PACE_SLICE_RATE);
			break;
		default:
			break;
	}
	draw_shpere(x, y, 1.0, 1.0, 0.0, 0.2, x_pace_fix, y_pace_fix);
}

#define ENEMY_1 1 /* RED */
#define ENEMY_2 2 /* CYAN */
#define ENEMY_3 3 /* PINK */
#define ENEMY_4 4 /* ORANGE */

void draw_enemy(int main_character_x, int main_character_y, int x, int y, int id)
{

	float x_pace_fix = 0.0, y_pace_fix = 0.0;
	float x_pace_fix_2 = 0.0, y_pace_fix_2 = 0.0;
	/* relative position from main character */
	switch (characters[0].direction)
	{
		case DIRECTION_UP:
			y_pace_fix = 0 - (float)pace / PACE_SLICE_RATE + 1;
			break;
		case DIRECTION_DOWN:
			y_pace_fix = +(float)pace / PACE_SLICE_RATE - 1;
			break;
		case DIRECTION_LEFT:
			x_pace_fix = ((float)pace / PACE_SLICE_RATE) - 1;
			break;
		case DIRECTION_RIGHT:
			x_pace_fix = 1 - ((float)pace / PACE_SLICE_RATE);
			break;
		default:
			break;
	}
	/* relative position from enemy character */
	switch (characters[id].direction)
	{
		case DIRECTION_UP:
			y_pace_fix_2 = (float)pace / PACE_SLICE_RATE;
			break;
		case DIRECTION_DOWN:
			y_pace_fix_2 = 1 - (float)pace / PACE_SLICE_RATE - 1;
			break;
		case DIRECTION_LEFT:
			x_pace_fix_2 = 1 - ((float)pace / PACE_SLICE_RATE) - 1;
			break;
		case DIRECTION_RIGHT:
			x_pace_fix_2 = ((float)pace / PACE_SLICE_RATE) + 1 - 1;
			break;
		default:
			break;
	}
	switch (id)
	{
		case ENEMY_1:
			draw_shpere(x - main_character_x, y - main_character_y, 1.0, 0.191, 0.0, 0.5, x_pace_fix + x_pace_fix_2, y_pace_fix + y_pace_fix_2);
			break;
		case ENEMY_2:
			draw_shpere(x - main_character_x, y - main_character_y, 0.0, 1.0, 0.9, 0.5, x_pace_fix + x_pace_fix_2, y_pace_fix + y_pace_fix_2);
			break;
		case ENEMY_3:
			draw_shpere(x - main_character_x, y - main_character_y, 1.0, 0.629, 0.8, 0.5, x_pace_fix + x_pace_fix_2, y_pace_fix + y_pace_fix_2);
			break;
		case ENEMY_4:
			draw_shpere(x - main_character_x, y - main_character_y, 1.0, 0.8, 0.0, 0.5, x_pace_fix + x_pace_fix_2, y_pace_fix + y_pace_fix_2);
			break;
		default:
			break;
	}
}

void draw_map()
{
	int x, y;
	for (y = 0; y < MAP_HEIGHT; y++) {
		for (x = 0; x < MAP_WIDTH; x++) {
			if (map_data[MAP_LOCATE(x, y)] == 1)
				draw_wall_block(x - characters[0].x, y - characters[0].y, characters[0].direction);
		}
	}
}

void draw_beans()
{
	int x, y;
	for (y = 0; y < MAP_HEIGHT; y++) {
		for (x = 0; x < MAP_WIDTH; x++) {
			if (map_data_working[MAP_LOCATE(x, y)] == 2)
				draw_bean(x - characters[0].x, y - characters[0].y, characters[0].direction);
		}
	}
}

void render()
{
	int i;
	glColor3f(0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	if (game_state == GAME_STATE_TITLE)
	{
		bmp_draw(bmp_title, 0, 0);
	}
	else
	{

		draw_map();
		draw_beans();
		draw_main_character(0, 0); for (i = 1; i <= 4; i++) { draw_enemy(characters[0].x, characters[0].y, characters[i].x, characters[i].y, i);
		}

		if (game_state == GAME_STATE_WIN)
		{
			bmp_draw(bmp_you_win, 0, 0);
		}
		else if (game_state == GAME_STATE_LOSE)
		{
			bmp_draw(bmp_you_lose, 0, 0);
		}
	}

	glFlush();
	glutSwapBuffers();
}

void init(int width, int height)
{
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective
		( 54.0f,
		  (GLfloat)width/(GLfloat)height,
		  0.1f,	
		  3000.0f
		);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	/* light */
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	/*glEnable(GL_CULL_FACE); */
	glFrontFace(GL_CCW);

	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLightR);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specularLightR);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPositionR);

	glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuseLightG);
	glLightfv(GL_LIGHT2, GL_SPECULAR, specularLightG);
	glLightfv(GL_LIGHT2, GL_POSITION, lightPositionG);

	glLightfv(GL_LIGHT3, GL_DIFFUSE, diffuseLightB);
	glLightfv(GL_LIGHT3, GL_SPECULAR, specularLightB);
	glLightfv(GL_LIGHT3, GL_POSITION, lightPositionB);

	glEnable(GL_LIGHT0); 
	glEnable(GL_LIGHT1); 
	glEnable(GL_LIGHT2); 
	glEnable(GL_LIGHT3); 
}

#if defined (UNIX)  
struct timeval last_time, current_time;  
#elif defined (WINDOWS)  
SYSTEMTIME last_time, current_time;  
#else  
time_t last_time, current_time;  
#endif  

void idle_callback()
{
	int i;
	int directions_for_enemy[4];
	int directions_for_enemy_count;
	long delta;
	if (game_state != GAME_STATE_RUNNING)
	{
		glutPostRedisplay();
		return;
	}
	/* get current time */
#if defined (UNIX)  
	gettimeofday(&current_time, NULL);  
#elif defined (WINDOWS)  
	GetSystemTime(&current_time);  
#else  
	time(&current_time);  
#endif  
	/* calculate delta */
#if defined (UNIX)  
	delta = (current_time.tv_sec * 1000000 + current_time.tv_usec) - (last_time.tv_sec * 1000000 + last_time.tv_usec);
#elif defined (WINDOWS)  
	delta = (current_time.wSecond * 1000000 + current_time.wMillisecond) - (last_time.wSecond * 1000000 + last_time.wMillisecond);  
#else  
	/*printf("time:%d/n", time2 - time1);  */
#endif  

	if (KEY_UP)
	{
		characters[0].next_direction = DIRECTION_NONE;
	}
	else if (KEY_DOWN)
	{
		if (key_state_up == KEY_STATE_DOWN)
		{
			if (map_data[MAP_LOCATE(characters[0].x, characters[0].y + 1)] != 1)
				characters[0].next_direction = DIRECTION_UP;
		}
		else if (key_state_down == KEY_STATE_DOWN)
		{
			if (map_data[MAP_LOCATE(characters[0].x, characters[0].y - 1)] != 1)
				characters[0].next_direction = DIRECTION_DOWN;
		}
		else if (key_state_left == KEY_STATE_DOWN)
		{
			if (map_data[MAP_LOCATE(characters[0].x - 1, characters[0].y)] != 1)
				characters[0].next_direction = DIRECTION_LEFT;
		}
		else if (key_state_right == KEY_STATE_DOWN)
		{
			if (map_data[MAP_LOCATE(characters[0].x + 1, characters[0].y)] != 1)
				characters[0].next_direction = DIRECTION_RIGHT;
		}
	}

	while (delta > PACE_SLICE_DELTA)
	{
		pace++;
		if (pace >= PACE_SLICE_RATE)
		{
			/* move main character */
			if (map_data_working[MAP_LOCATE(characters[0].x, characters[0].y)] == MAP_BEAN)
			{
				map_data_working[MAP_LOCATE(characters[0].x, characters[0].y)] = MAP_WALL;
				beans_eat++;
				if (beans_eat == beans_total)
				{
					game_state = GAME_STATE_WIN;
				}
			}
			pace = 0;
			switch (characters[0].next_direction)
			{
				case DIRECTION_UP:
					characters[0].y++;
					break;
				case DIRECTION_DOWN:
					characters[0].y--;
					break;
				case DIRECTION_LEFT:
					characters[0].x--;
					break;
				case DIRECTION_RIGHT:
					characters[0].x++;
					break;
			}
			for (i = 1; i <= 4; i++)
			{
				if (characters[i].x == characters[0].x && characters[i].y == characters[0].y)
				{
					game_state = GAME_STATE_LOSE;
					key_state_up = key_state_down = key_state_left = key_state_right = KEY_STATE_UP;
				}
			}

			characters[0].direction = characters[0].next_direction;
			if ((characters[0].next_direction == DIRECTION_UP && map_data_working[MAP_LOCATE(characters[0].x, characters[0].y - 1)] == MAP_WALL) || 
					(characters[0].next_direction == DIRECTION_DOWN && map_data_working[MAP_LOCATE(characters[0].x, characters[0].y + 1)] == MAP_WALL) || 
					(characters[0].next_direction == DIRECTION_LEFT && map_data_working[MAP_LOCATE(characters[0].x + 1, characters[0].y)] == MAP_WALL) || 
					(characters[0].next_direction == DIRECTION_RIGHT && map_data_working[MAP_LOCATE(characters[0].x - 1, characters[0].y)] == MAP_WALL))
			{
				characters[0].next_direction = DIRECTION_NONE;
			}
			/* move enemy */
			for (i = 1; i <= 4; i++)
			{
				switch (characters[i].direction)
				{
					case DIRECTION_UP:
						characters[i].y++;
						break;
					case DIRECTION_DOWN:
						characters[i].y--;
						break;
					case DIRECTION_LEFT:
						characters[i].x--;
						break;
					case DIRECTION_RIGHT:
						characters[i].x++;
						break;
				}
				if (characters[i].x == characters[0].x && characters[i].y == characters[0].y)
				{
					game_state = GAME_STATE_LOSE;
				}
				/* count possible directions */
				directions_for_enemy_count = 0;
				if (map_data[MAP_LOCATE(characters[i].x, characters[i].y + 1)] != MAP_WALL)
				{
					directions_for_enemy[directions_for_enemy_count] = DIRECTION_UP;
					directions_for_enemy_count++;
				}
				if (map_data[MAP_LOCATE(characters[i].x, characters[i].y - 1)] != MAP_WALL)
				{
					directions_for_enemy[directions_for_enemy_count] = DIRECTION_DOWN;
					directions_for_enemy_count++;
				}
				if (map_data[MAP_LOCATE(characters[i].x - 1, characters[i].y)] != MAP_WALL)
				{
					directions_for_enemy[directions_for_enemy_count] = DIRECTION_LEFT;
					directions_for_enemy_count++;
				}
				if (map_data[MAP_LOCATE(characters[i].x + 1, characters[i].y)] != MAP_WALL)
				{
					directions_for_enemy[directions_for_enemy_count] = DIRECTION_RIGHT;
					directions_for_enemy_count++;
				}
				/* redetermine direction */
				if (directions_for_enemy_count == 1)
				{
					switch (characters[i].direction)
					{
						case DIRECTION_UP:
							characters[i].direction = DIRECTION_DOWN;
							break;
						case DIRECTION_DOWN:
							characters[i].direction = DIRECTION_UP;
							break;
						case DIRECTION_LEFT:
							characters[i].direction = DIRECTION_RIGHT;
							break;
						case DIRECTION_RIGHT:
							characters[i].direction = DIRECTION_LEFT;
							break;
					}
				}
				else
				{
					directions_for_enemy_count = 0;
					if (map_data[MAP_LOCATE(characters[i].x, characters[i].y + 1)] != MAP_WALL && characters[i].direction != DIRECTION_DOWN)
					{
						directions_for_enemy[directions_for_enemy_count] = DIRECTION_UP;
						directions_for_enemy_count++;
					}
					if (map_data[MAP_LOCATE(characters[i].x, characters[i].y - 1)] != MAP_WALL && characters[i].direction != DIRECTION_UP)
					{
						directions_for_enemy[directions_for_enemy_count] = DIRECTION_DOWN;
						directions_for_enemy_count++;
					}
					if (map_data[MAP_LOCATE(characters[i].x - 1, characters[i].y)] != MAP_WALL && characters[i].direction != DIRECTION_RIGHT)
					{
						directions_for_enemy[directions_for_enemy_count] = DIRECTION_LEFT;
						directions_for_enemy_count++;
					}
					if (map_data[MAP_LOCATE(characters[i].x + 1, characters[i].y)] != MAP_WALL && characters[i].direction != DIRECTION_LEFT)
					{
						directions_for_enemy[directions_for_enemy_count] = DIRECTION_RIGHT;
						directions_for_enemy_count++;
					}

				}
				characters[i].direction = directions_for_enemy[rand() % directions_for_enemy_count];
			}
		}
		delta -= PACE_DELTA;
		last_time = current_time;
	}
	glutPostRedisplay();
}

void gamelogic_init();
void process_key_down(unsigned char key, int x, int y) 
{ 
	if (game_state == GAME_STATE_TITLE)
	{
		if (key == 13) 
		{
			gamelogic_init();
			/* time counter */
#if defined (UNIX)  
			gettimeofday(&last_time, NULL);  
#elif defined (WINDOWS)  
			GetSystemTime(&last_time);  
#else  
			time(&last_time);  
#endif  
			game_state = GAME_STATE_RUNNING;
		}
	}
	else if (game_state == GAME_STATE_RUNNING)
	{
		switch (key)
		{
			case 'w':
				key_state_up = KEY_STATE_DOWN;
				break;
			case 's':
				key_state_down = KEY_STATE_DOWN;
				break;
			case 'a':
				key_state_left = KEY_STATE_DOWN;
				break;
			case 'd':
				key_state_right = KEY_STATE_DOWN;
				break;
		}
	}
	else if (game_state == GAME_STATE_LOSE)
	{
		if (key == 13) 
		{
			game_state = GAME_STATE_TITLE;
			gamelogic_init();
		}
	}
} 

void process_key_up(unsigned char key, int x, int y) 
{ 
	switch (key)
	{
		case 'w':
			key_state_up = KEY_STATE_UP;
			break;
		case 's':
			key_state_down = KEY_STATE_UP;
			break;
		case 'a':
			key_state_left = KEY_STATE_UP;
			break;
		case 'd':
			key_state_right = KEY_STATE_UP;
			break;
	}
}

void gamelogic_init()
{
	characters[0].x = 10;
	characters[0].y = 1;
	characters[0].direction = DIRECTION_NONE;

	characters[1].x = 9;
	characters[1].y = 5;
	characters[1].direction = DIRECTION_NONE;

	characters[2].x = 11;
	characters[2].y = 5;
	characters[2].direction = DIRECTION_NONE;

	characters[3].x = 9;
	characters[3].y = 7;
	characters[3].direction = DIRECTION_NONE;

	characters[4].x = 11;
	characters[4].y = 7;
	characters[4].direction = DIRECTION_NONE;

	memcpy(map_data_working, map_data, sizeof(map_data));

	srand(time(NULL)); 
	unsigned char *map_data_p = map_data, *map_data_endp = map_data + sizeof(map_data);
	beans_total = 0;
	while (map_data_p != map_data_endp)
	{
		if (*map_data_p == MAP_BEAN) beans_total++;
		map_data_p++;
	}
	beans_eat = 0;
}

int main(int argc, char *argv[])
{
	/* game state */
	game_state = GAME_STATE_TITLE;

	/* init part 1 */
	glutInit(&argc, (char **)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - WINDOW_WIDTH) / 2,  
			(glutGet(GLUT_SCREEN_HEIGHT) - WINDOW_HEIGHT) / 2);  
	glutCreateWindow("Pacman 3D");
	init(WINDOW_WIDTH, WINDOW_HEIGHT);
	/* game logic init */
	gamelogic_init();
	/* init resources */
	load_resource();

	glutKeyboardUpFunc(process_key_up); 
	glutKeyboardFunc(process_key_down); 
	glutIdleFunc(idle_callback);

	glutDisplayFunc(render);

	/* glut's main loop */
	glutMainLoop();

	return 0;
}

