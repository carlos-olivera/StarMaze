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
 *    Game framework                                                     *
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
#ifdef _MSC_VER
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif
#include "gp.h"
#include "SFont.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void collectiblecollision()
	{
	int i;
	for (i = 0; i < gCollectibleCount; i++)
		{
		if (gCollectible[i].mTaken == 0)
			{
			if (sqrt((gCollectible[i].mX - gXPos)
					* (gCollectible[i].mX - gXPos) + (gCollectible[i].mY
					- gYPos) * (gCollectible[i].mY - gYPos))
					< RADIUS + gCollectible[i].mRadius)
				{
				gCollectiblesTaken++;
				gCollectible[i].mTaken = 1;
				gScore += 10;
				}
			}
		}
	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void reset()
	{
	gXMov = 0;
	gYMov = 0;

	gKeyLeft = 0;
	gKeyRight = 0;
	gKeyUp = 0;
	gKeyDown = 0;

	char name[80];
	FILE * f;
	do
		{
		sprintf(name, TXT_LEVEL, gCurrentLevel);
		f = fopen(name, "rb");
		if (f == NULL)
			{
			if (gCurrentLevel == 0)
				exit(0);
			gCurrentLevel = 0;
			}
		}
	while (f == NULL);

	gLevelWidth = -1;
	int i = 0;
	while (i != '\n' && i != '\r')
		{
		i = fgetc(f);
		gLevelWidth++;
		}
	gLevelHeight = 1;
	int ch = 0;
	while (!feof(f) && ch != '@')
		{
		ch = fgetc(f);
		if (ch == i)
			gLevelHeight++;
		}
	fseek(f, 0, SEEK_SET);

	delete[] gLevel;
	gLevel = new unsigned char[gLevelWidth * gLevelHeight];

	delete[] gColliders;
	gColliders = new unsigned char[gLevelWidth * gLevelHeight];
	memset(gColliders, 0, sizeof(unsigned char) * gLevelWidth * gLevelHeight);

	int p = 0;
	while (p < gLevelWidth * gLevelHeight && !feof(f))
		{
		int v = fgetc(f);
		if (v > 32)
			{
			switch (v)
				{
				case '.':
					gLevel[p] = LEVEL_DROP;
					break;
				case '_':
					gLevel[p] = LEVEL_GROUND;
					break;
				case 'S':
					gLevel[p] = LEVEL_START;
					break;
				case 'E':
					gLevel[p] = LEVEL_END;
					break;
				case 'o':
					gLevel[p] = LEVEL_COLLECTIBLE;
					break;
				case '>':
					gLevel[p] = LEVEL_RIGHT;
					break;
				case '<':
					gLevel[p] = LEVEL_LEFT;
					break;
				case 'v':
					gLevel[p] = LEVEL_DOWN;
					break;
				case '^':
					gLevel[p] = LEVEL_UP;
					break;
				case '#':
					gLevel[p] = LEVEL_WALL;
					break;
				case '~':
					gLevel[p] = LEVEL_ROUGH;
					break;
				case '=':
					gLevel[p] = LEVEL_SMOOTH;
					break;
				}
			p++;
			}
		}

	delete[] gLevelName;
	gLevelName = NULL;
	gLevelTime = TIMELIMIT;

	ch = 0;
	while (!feof(f) && ch != '@')
		ch = fgetc(f);
	if (ch == '@')
		{

		i = 0;
		ch = 0;
		while (!feof(f) && ch != '@')
			{
			ch = fgetc(f);
			name[i] = ch;
			i++;
			}
		name[i-1] = 0;
		gLevelTime = atoi(name);

		i = 0;
		ch = 0;
		while (!feof(f) && ch != '@')
			{
			ch = fgetc(f);
			name[i] = ch;
			i++;
			}
		name[i-1] = 0;

		}

	int len = (int)strlen(name);
	gLevelName = new char[len + 1];
	memcpy(gLevelName, name, len + 1);

	fclose(f);

	gCollectibleCount = 0;
	for (i = 0; i < gLevelWidth * gLevelHeight; i++)
		{
		if (gLevel[i] == LEVEL_START)
			{
			gStartX = (float)((i % gLevelWidth) * TILESIZE + TILESIZE / 2);
			gStartY = (float)((i / gLevelWidth) * TILESIZE + TILESIZE / 2);
			}
		if (gLevel[i] == LEVEL_COLLECTIBLE)
			gCollectibleCount++;

		if (gLevel[i] == LEVEL_WALL)
			{
			int ypos = i / gLevelWidth;
			int xpos = i % gLevelWidth;

			if (ypos > 0)
				{
				if (xpos > 0)
					gColliders[i - gLevelWidth - 1] |= COLLIDE_SE;

				gColliders[i - gLevelWidth] |= COLLIDE_S;

				if (xpos < gLevelWidth - 1)
					gColliders[i - gLevelWidth + 1] |= COLLIDE_SW;
				}
			if (xpos > 0)
				gColliders[i - 1] |= COLLIDE_E;

			if (xpos < gLevelWidth - 1)
				gColliders[i + 1] |= COLLIDE_W;

			if (ypos < gLevelHeight - 1)
				{
				if (xpos > 0)
					gColliders[i + gLevelWidth - 1] |= COLLIDE_NE;

				gColliders[i + gLevelWidth] |= COLLIDE_N;

				if (xpos < gLevelWidth - 1)
					gColliders[i + gLevelWidth + 1] |= COLLIDE_NW;
				}
			}
		}

	delete[] gCollectible;
	gCollectible = new collectible[gCollectibleCount];
	gCollectibleCount = 0;
	for (i = 0; i < gLevelWidth * gLevelHeight; i++)
		{
		if (gLevel[i] == LEVEL_COLLECTIBLE)
			{
			gCollectible[gCollectibleCount].mX = (float)((i % gLevelWidth)
					* TILESIZE + TILESIZE / 2);
			gCollectible[gCollectibleCount].mY = (float)((i / gLevelWidth)
					* TILESIZE + TILESIZE / 2);
			gCollectible[gCollectibleCount].mColor = COLLECTIBLECOLOR;
			gCollectible[gCollectibleCount].mRadius = COLLECTIBLERADIUS;
			gCollectible[gCollectibleCount].mTaken = 0;
			gCollectibleCount++;
			}
		}

	gCollectiblesTaken = 0;

	gXPos = gStartX;
	gYPos = gStartY;

	gLastTick = SDL_GetTicks();
	gLevelStartTick = SDL_GetTicks();

	gCameraX = (screenWIDTH / 2) - gXPos;
	gCameraY = (screenHEIGHT / 2) - gYPos;

	}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void rendergame()
	{
	// Ask SDL for the time in milliseconds
	int tick = SDL_GetTicks();

	if (tick <= gLastTick)
		{
		SDL_Delay(1);
		return;
		}

	while (gLastTick < tick)
		{

		int currenttile = (((int)gYPos) / TILESIZE) * gLevelWidth
				+ ((int)gXPos) / TILESIZE;

		if (gKeyLeft)
			gXMov -= THRUST;
		if (gKeyRight)
			gXMov += THRUST;
		if (gKeyUp)
			gYMov -= THRUST;
		if (gKeyDown)
			gYMov += THRUST;

		switch (gLevel[currenttile])

			{
			case LEVEL_DROP:
				// player fell off - reset position
				reset();
				break;
			case LEVEL_END:
				{
				int secondsleft = gLevelTime - (gLastTick - gLevelStartTick)
						/ 1000;
				if (secondsleft < 0)
					secondsleft = 0;
				gScore += 100 + secondsleft * 25;
				gCurrentLevel++;
				reset();
				}
				break;

			case LEVEL_LEFT:
				gXMov -= SLIDEPOWER;
				break;
			case LEVEL_RIGHT:
				gXMov += SLIDEPOWER;
				break;
			case LEVEL_UP:
				gYMov -= SLIDEPOWER;
				break;
			case LEVEL_DOWN:
				gYMov += SLIDEPOWER;
				break;
			}

		switch (gLevel[currenttile])
			{
			case LEVEL_SMOOTH:
				gXPos += gXMov;
				gYPos += gYMov;
				break;
			case LEVEL_ROUGH:
				gXMov *= SLOWDOWNROUGH;
				gYMov *= SLOWDOWNROUGH;
				gXPos += gXMov;
				gYPos += gYMov;
				break;
			default:
				gXMov *= SLOWDOWN;
				gYMov *= SLOWDOWN;
				gXPos += gXMov;
				gYPos += gYMov;
			}

		float targetx = (screenWIDTH / 2) - (gXPos + gXMov * 25);
		float targety = (screenHEIGHT / 2) - (gYPos + gYMov * 25);
		gCameraX = (targetx + gCameraX * 19) / 20;
		gCameraY = (targety + gCameraY * 19) / 20;

		// Collision with tiles
		currenttile = (int)(gYPos / TILESIZE) * gLevelWidth + (int)(gXPos
				/ TILESIZE);
		float xposintile = gXPos - (int)(gXPos / TILESIZE) * TILESIZE;
		float yposintile = gYPos - (int)(gYPos / TILESIZE) * TILESIZE;

		int collision = 0;
		float normalx = 0;
		float normaly = 0;

		// Check collision with tile edges

		if (gColliders[currenttile] & COLLIDE_E && xposintile
				> (TILESIZE - RADIUS))
			{
			normalx -= xposintile - (TILESIZE - RADIUS);
			collision = 1;
			}

		if (gColliders[currenttile] & COLLIDE_W && xposintile < (RADIUS))
			{
			normalx += (RADIUS) - xposintile;
			collision = 1;
			}

		if (gColliders[currenttile] & COLLIDE_S && yposintile
				> (TILESIZE - RADIUS))
			{
			normaly -= yposintile - (TILESIZE - RADIUS);
			collision = 1;
			}

		if (gColliders[currenttile] & COLLIDE_N && yposintile < (RADIUS))
			{
			normaly += (RADIUS) - yposintile;
			collision = 1;
			}

		if (collision)
			{
			gXPos += normalx;
			gYPos += normaly;

			// re-calculate the positions so that we don't collide with
			// corners unneccessarily after colliding with the walls.
			xposintile = gXPos - (int)(gXPos / TILESIZE) * TILESIZE;
			yposintile = gYPos - (int)(gYPos / TILESIZE) * TILESIZE;
			}

		// Check collision with tile corners

		if (gColliders[currenttile] & COLLIDE_NE && ((xposintile - 32)
				* (xposintile - 32) + (yposintile * yposintile))
				< (RADIUS * RADIUS))
			{
			float dist = (float)sqrt((xposintile - 32) * (xposintile - 32)
					+ yposintile * yposintile);
			if (dist > 0)
				{
				normalx += (RADIUS - xposintile) / dist;
				normalx += (RADIUS - yposintile) / dist;
				collision = 1;
				}
			}

		if (gColliders[currenttile] & COLLIDE_NW && ((xposintile)
				* (xposintile) + (yposintile * yposintile)) < (RADIUS * RADIUS))
			{
			float dist = (float)sqrt(xposintile * xposintile + yposintile
					* yposintile);
			if (dist > 0)
				{
				normalx += (RADIUS - xposintile) / dist;
				normaly += (RADIUS - yposintile) / dist;
				collision = 1;
				}
			}

		if (gColliders[currenttile] & COLLIDE_SE && ((xposintile - 32)
				* (xposintile - 32) + ((yposintile -32) * (yposintile - 32)))
				< (RADIUS * RADIUS))
			{
			float dist = (float)sqrt((xposintile - 32) * (xposintile - 32)
					+ (yposintile - 32) * (yposintile - 32));
			if (dist > 0)
				{
				normalx += (RADIUS - xposintile) / dist;
				normaly += (RADIUS - yposintile) / dist;
				collision = 1;
				}
			}

		if (gColliders[currenttile] & COLLIDE_SW && ((xposintile)
				* (xposintile) + ((yposintile -32) * (yposintile - 32)))
				< (RADIUS * RADIUS))
			{
			float dist = (float)sqrt(xposintile * xposintile
					+ (yposintile - 32) * (yposintile - 32));
			if (dist > 0)
				{
				normalx += (RADIUS - xposintile) / dist;
				normaly += (RADIUS - yposintile) / dist;
				collision = 1;
				}
			}

		if (collision)
			{
			gXPos += normalx;
			gYPos += normaly;

			// Normalize (i.e. make unit length) the collision normal
			float len = sqrt(normalx*normalx + normaly*normaly);
			normalx /= len;
			normaly /= len;

			// Calculate dot product between the wall collision and motion vector
			float dot = gXMov * normalx + gYMov * normaly;

			// Adjust the motion vector based on the collision
			gXMov -= dot * 1.25f * normalx;
			gYMov -= dot * 1.25f * normaly;

			}

		// Collision with the level borders
		if (gXPos > gLevelWidth * TILESIZE || 
	        gXPos < 0 || 
	        gYPos > gLevelHeight * TILESIZE || 
	        gYPos < 0)
{
			reset();
			}

					collectiblecollision();

		gLastTick += 1000 / PHYSICSFPS;
		}

	// Lock surface if needed
	if (SDL_MUSTLOCK(gScreen))
		if (SDL_LockSurface(gScreen) < 0)
			return;

	// fill background
	scrollStars();
	renderStars();

	int i, j;
	for (i = 0; i < gLevelHeight; i++)
		{
		for (j = 0; j < gLevelWidth; j++)
			{
			if (gLevel[i * gLevelWidth + j] != 0)
				{
				int tile = 0;

				switch (gLevel[i * gLevelWidth + j])
					{
					case LEVEL_START:
						tile = 2;
						break;
					case LEVEL_END:
						tile = 1;
						break;
					case LEVEL_UP:
						tile = 4;
						break;
					case LEVEL_RIGHT:
						tile = 5;
						break;
					case LEVEL_DOWN:
						tile = 6;
						break;
					case LEVEL_LEFT:
						tile = 7;
						break;
					case LEVEL_WALL:
						tile = 3;
						break;
					case LEVEL_ROUGH:
						tile = 8;
						break;
					case LEVEL_SMOOTH:
						tile = 9;
						break;
					case LEVEL_GROUND:
					case LEVEL_COLLECTIBLE:
					default:
						// tile = 0;
						break;
					}
				drawtile((int)(j * TILESIZE + gCameraX), (int)(i * TILESIZE
						+ gCameraY), tile);
				}
			}
		}

	// draw the collectibles
	for (i = 0; i < gCollectibleCount; i++)
		{
		if (gCollectible[i].mTaken == 0)
			{
			drawcircle((int)(gCollectible[i].mX + 2 + gCameraX),
					(int)(gCollectible[i].mY + 2 + gCameraY),
					gCollectible[i].mRadius, 0);
			drawcircle((int)gCollectible[i].mX + gCameraX,
					(int)gCollectible[i].mY + gCameraY,
					gCollectible[i].mRadius, gCollectible[i].mColor);

			}
		}

	// draw the player object
	//drawcircle((int)(gXPos + gCameraX)+2, (int)(gYPos + gCameraY)+3, 
	//	RADIUS, SDL_MapRGB(gScreen->format, 90, 77, 80));
	draw_ball((int)(gXPos + gCameraX)-RADIUS, (int)(gYPos + gCameraY)-RADIUS);

	// draw status strings
	char scorestring[80];
	char statusstring[80];
	sprintf(statusstring, "'%s', time limit:%ds", gLevelName, gLevelTime);
	SFont_Write(gScreen, gFont, 5, 5, statusstring);

	sprintf(statusstring, "Score:%d", gScore);
	SFont_Write(gScreen, gFont, 5, 22, statusstring);

	int secondsleft = gLevelTime - (gLastTick - gLevelStartTick) / 1000;
	if (secondsleft < 0)
		secondsleft = 0;
	sprintf(statusstring, "Time:%d", secondsleft);
	SFont_Write(gScreen, gFont, 5, 39, statusstring);

	sprintf(scorestring, "FPS:%d", displayFrames);
	SFont_Write(gScreen, gFont, 5, 55, scorestring);

	// Unlock if needed
	if (SDL_MUSTLOCK(gScreen))
		SDL_UnlockSurface(gScreen);

	// Tell SDL to update the whole gScreen
	SDL_UpdateRect(gScreen, 0, 0, screenWIDTH, screenHEIGHT);

	//if time over
	if (secondsleft <= 0)
		reset();
	}
