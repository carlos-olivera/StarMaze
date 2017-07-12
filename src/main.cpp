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

/************************************************************************ 
*    Main Game Code                                                     *
*    Based on SDL tutorial created by:                                  *
*       Jari Komppa (http://sol.gfxile.net/), thanks!                   *
*    And thanks to: Markus Mertama for his work on:                     *
*       SDL for S60 3rd edition and Symbian OS library                  *
*       http://koti.mbnet.fi/mertama/sdl.html                           *
*                                                                       *
************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sdl.h>
#include "gp.h"
#include "SFont.h"

// Screen width
int screenWIDTH;
// Screen height
int screenHEIGHT;
// Screen surface
SDL_Surface *gScreen;
// Surface that contains the tiles
SDL_Surface *gTiles;
// Surface that contains the font
SFont_Font *gFont;
// Surface that contains the ball
SDL_Surface *gBall;

// video flags symbian
int videoFlags= SDL_SWSURFACE | SDL_ANYFORMAT;

// Total number of collectibles
int gCollectibleCount;
// Number of collectibles taken
int gCollectiblesTaken;
// Array of collectibles
collectible *gCollectible;
// Last iteration's tick value
int gLastTick;
// Level start tick
int gLevelStartTick;
// Current level
int gCurrentLevel;

// Player's position
float gXPos;
float gYPos;

// Player's motion vector
float gXMov;
float gYMov;

// Player's start position
float gStartX;
float gStartY;

// Player's key status
int gKeyLeft;
int gKeyRight;
int gKeyUp;
int gKeyDown;

// Player's score
int gScore;

// Level data
unsigned char *gLevel;

// Camera position
float gCameraX;
float gCameraY;

// Done
int Done = 0;
int displayFrames;

// Dynamic level width and height, in tiles
int gLevelWidth;
int gLevelHeight;

// Level collision data
unsigned char *gColliders;

// Level name string
char *gLevelName;
// Level time limit
int gLevelTime;

//*********************
void panic(char* aWhen)
	{
	fprintf(stderr, "SDL error: %s: %s\n", aWhen, SDL_GetError());
	SDL_Quit();
	exit(-1);
	}

void init()
	{
	gLevel = NULL;
	gCollectible = NULL;
	gScore = 0;
	gCurrentLevel = 0;
	gColliders = NULL;
	gLevelName = NULL;
	initStars();
	g_bScrollLeft = true;

	gTiles = SDL_LoadBMP(BMP_TILES);
	if (gTiles == NULL)
		panic("image tiles.bmp not found");

	gFont = SFont_InitFont(SDL_LoadBMP(BMP_FONT));
	if (!gFont)
		{
		panic("image font.bmp not found");
		}

	ball_sprite(BMP_BALL, 32, 32);

	reset();
	}

void render()
	{
	rendergame();
	}

// Entry point
int main(int argc, char *argv[])
	{
	// Initialize SDL's subsystems
	if (SDL_Init( SDL_INIT_VIDEO) < 0)
		{
		panic("SDL Init");
		}

	screenWIDTH = 320;
	screenHEIGHT = 240;

	SDL_Rect **modes;

	modes = SDL_ListModes(NULL, SDL_FULLSCREEN); //SDL_FULLSCREEN
	if (modes == NULL)
		{
		}
	if (modes != (SDL_Rect**) -1)
		{
		screenWIDTH = modes[0]->w;
		screenHEIGHT = modes[0]->h;
		}

	// Register SDL_Quit to be called at exit; makes sure things are
	// cleaned up when we quit.
	atexit(SDL_Quit);

	// Attempt to create a WIDTHxHEIGHT window with 32bit pixels.
	gScreen = SDL_SetVideoMode(screenWIDTH, screenHEIGHT, 32, videoFlags);

	init();

	// If we fail, return error.
	if (gScreen == NULL)
		{
		panic("No available video modes");
		}

	int initialTime = SDL_GetTicks();
	int finalTime = SDL_GetTicks();
	int frames = 0;

	// Main loop
	while (!Done)
		{
		// Render stuff
		render();

		// Poll for events, and handle the ones we care about.
		SDL_Event event;
		while (SDL_PollEvent(&event))
			{
			switch (event.type)
				{
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
						{
						case SDLK_LEFT:
							gKeyLeft = 1;
							break;
						case SDLK_RIGHT:
							gKeyRight = 1;
							break;
						case SDLK_UP:
							gKeyUp = 1;
							break;
						case SDLK_DOWN:
							gKeyDown = 1;
							break;
						}
					break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
						{
						case SDLK_ESCAPE:
							// If escape is pressed, return (and thus, quit)
							return 0;
						case SDLK_LEFT:
							gKeyLeft = 0;
							break;
						case SDLK_RIGHT:
							gKeyRight = 0;
							break;
						case SDLK_UP:
							gKeyUp = 0;
							break;
						case SDLK_DOWN:
							gKeyDown = 0;
							break;
						}
					break;
				case SDL_QUIT:
					Done = 1;
				}
			}

		finalTime = SDL_GetTicks();
		++frames;
		if ((finalTime - initialTime) >= 1000)
			{
			displayFrames = frames;
			frames = 0;
			initialTime = finalTime;
			}

		}
	SDL_FreeSurface(gBall);
	SFont_FreeFont(gFont);
	SDL_FreeSurface(gTiles);
	SDL_FreeSurface(gScreen);
	SDL_Quit();
	return 0;
	}
