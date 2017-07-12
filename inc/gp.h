/*  StarMaze: Maze 2D game for Symbian C++ 
    Copyright (C) 2008 Carlos Olivera Terrazas (carlos.olivera.t@gmail.com)

    License: GPL or LGPL (at your choice)
    WWW: http://www.series60.com.ar/blog/index.php/starmaze/

    This program is free software; you can redistribute it and/or modify        
    it under the terms of the GNU General Public License as published by        
    the Free Software Foundation; either version 2 of the License, or           
    (at your option) any later version.                                         
                                                                                
    This program is distributed in the hope that it will be useful,       
    but WITHOUT ANY WARRANTY; without even the implied warranty of              
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               
    GNU General Public License for more details.                
                                                                               
    You should have received a copy of the GNU General Public License           
    along with this program; if not, write to the Free Software                 
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA   
                                                                                                                                      
*/   


// include SFont library (http://user.cs.tu-berlin.de/~karlb/sfont/) 
// 
#include "SFont.h"

// Screen pitch
#define PITCH (gScreen->pitch / 4)
// Radius of the player's ball
#define RADIUS 15
// Background color
#define BGCOLOR 0x5f0000
// Definition of PI
#define PI 3.1415926535897932384626433832795f
// Fall color
#define FALLCOLOR 0x000000
// Tile size (width and height)
#define TILESIZE 32
// How many seconds does the player have to solve the level
#define TIMELIMIT 15
// Player's thrust value
#define THRUST 0.1f
// Sliding tile thrust power
#define SLIDEPOWER 0.04f
// Slowdown due to friction, etc.
#define SLOWDOWN 0.99f
// Slowdown due to collision
#define COLLISIONSLOWDOWN 0.9f
// Slowdown due to rough tile
#define SLOWDOWNROUGH 0.95f
// Color of the start tile
#define STARTCOLOR 0x001f7f
// Color of the goal tile
#define ENDCOLOR 0x3f7f1f
// Radius of a collectible item
#define COLLECTIBLERADIUS 8
// Color of a collectible item
#define COLLECTIBLECOLOR 0xffff00
// Physics iterations per second
#define PHYSICSFPS 100
// Color of player's ball
#define BALLCOLOR 0x003f9f


//-----------------------------------------------------------------------------
// Stars background
//-----------------------------------------------------------------------------
const int TOTAL_STARS     = 100;
const int UPDATE_INTERVAL = 30;

/////////////////////////////////////////////////////////////////////
// Enumerations

// Tile values
enum leveldataenum
{
  LEVEL_DROP = 0,
  LEVEL_GROUND = 1,
  LEVEL_START = 2,
  LEVEL_END = 3,
  LEVEL_COLLECTIBLE = 5,
  LEVEL_LEFT = 10,
  LEVEL_RIGHT = 11,
  LEVEL_UP = 12,
  LEVEL_DOWN = 13,
  LEVEL_WALL = 24,
  LEVEL_SMOOTH = 25,
  LEVEL_ROUGH = 26

};

// Tile collision directions
enum colliderenum
{
  COLLIDE_N  = 0x01,
  COLLIDE_NW = 0x02,
  COLLIDE_W  = 0x04,
  COLLIDE_SW = 0x08,
  COLLIDE_S  = 0x10,
  COLLIDE_SE = 0x20,
  COLLIDE_E  = 0x40,
  COLLIDE_NE = 0x80
};


/////////////////////////////////////////////////////////////////////
// Structures

// Vertex structure
struct vertex
{
  float x, y, z;
};


struct collectible
{
  float mX;
  float mY;
  int mColor;
  int mRadius;
  int mTaken;
};

enum
	{
	ETestDsa = 0x5000,
	ETestDsbDb,
	ETestDsbIu,
	ETestDsbDbA,
	ETestDsbIuA,
	ETestGdi
	};

/////////////////////////////////////////////////////////////////////
// Globals

// Screen width
extern int screenWIDTH;
// Screen height
extern int screenHEIGHT;
// Screen surface
extern SDL_Surface *gScreen;
// Surface that contains the tiles
extern SDL_Surface *gTiles;
// Surface that contains the font
extern SFont_Font *gFont;
// Surface that contains the ball
extern SDL_Surface *gBall;
// Total number of collectibles
extern int gCollectibleCount;
// Number of collectibles taken
extern int gCollectiblesTaken;
// Array of collectibles
extern collectible *gCollectible;
// Last iteration's tick value
extern int gLastTick;
// Level start tick
extern int gLevelStartTick;
// Current level
extern int gCurrentLevel;
// Player's position
extern float gXPos;
extern float gYPos;
// Player's motion vector
extern float gXMov;
extern float gYMov;
// Player's start position
extern float gStartX;
extern float gStartY;
// Player's key status
extern int gKeyLeft;
extern int gKeyRight;
extern int gKeyUp;
extern int gKeyDown;
// Player's score
extern int gScore;
// Level data
extern unsigned char *gLevel;
// Camera position
extern float gCameraX;
extern float gCameraY;
// back. starts direction
extern bool g_bScrollLeft;
// FPS calculation
extern int displayFrames;
// Dynamic level width and height, in tiles
extern int gLevelWidth;
extern int gLevelHeight;
// Level collision data
extern unsigned char *gColliders; 
// Level name string
extern char *gLevelName;
// Level time limit
extern int gLevelTime;

/////////////////////////////////////////////// strings
#ifdef __WINS__
    const char* const BMP_TILES = "z:\\private\\03513A94\\tiles.bmp";
    const char* const BMP_FONT = "z:\\private\\03513A94\\font.bmp";
    const char* const BMP_BALL = "z:\\private\\03513A94\\ball.bmp";
    const char* const TXT_LEVEL = "z:\\private\\03513A94\\level%d.txt";

 #else
    const char* const BMP_TILES = "tiles.bmp";
    const char* const BMP_FONT = "font.bmp";  
    const char* const BMP_BALL = "ball.bmp";
    const char* const TXT_LEVEL = "level%d.txt";
  #endif

/////////////////////////////////////////////////////////////////////
// Functions

extern void collectiblecollision();
extern void reset();
extern void rendergame();
extern void panic(char* aWhen);
extern void drawcircle(int x, int y, int r, int c);
extern void drawrect(int x, int y, int width, int height, int c);
extern void drawtile(int x, int y, int tile);
extern void ball_sprite(const char *filename, int sw, int sh);
extern void draw_ball(int x, int y);
extern void initStars( void );
extern void scrollStars(void);
extern void renderPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
extern void renderStars(void);
