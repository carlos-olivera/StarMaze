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
*    Graphics functions                                                 *
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
#include "SFont.h"
#ifdef _MSC_VER
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif
#include "gp.h"

//************************* backgound stars code
bool g_bScrollLeft = false;

struct STAR
{
    int x;        // Star posit x
    int y;        // Star posit y
    int velocity; // Star velocity
};

STAR stars[TOTAL_STARS]; // Star field array
//*************************


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void drawcircle(int x, int y, int r, int c)
{
  int i, j;
  for (i = 0; i < 2 * r; i++)
  {
        // vertical clipping: (top and bottom)
    if ((y - r + i) >= 0 && (y - r + i) < screenHEIGHT)
    {
      int len = (int)sqrt((float)(r * r - (r - i) * (r - i))) * 2;
      int xofs = x - len / 2;

      // left border
      if (xofs < 0)
      {
        len += xofs;
        xofs = 0;
      }

      // right border
      if (xofs + len >= screenWIDTH)
      {
        len -= (xofs + len) - screenWIDTH;
      }
      int ofs = (y - r + i) * PITCH + xofs;
      
      // note that len may be 0 at this point, 
      // and no pixels get drawn!
      for (j = 0; j < len; j++)
        ((unsigned int*)gScreen->pixels)[ofs + j] = c;
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void drawrect(int x, int y, int width, int height, int c)
{
  int i, j;
  for (i = 0; i < height; i++)
  {
    // vertical clipping: (top and bottom)
    if ((y + i) >= 0 && (y + i) < screenHEIGHT)
    {
      int len = width;
      int xofs = x;

      // left border
      if (xofs < 0)
      {
        len += xofs;
        xofs = 0;
      }

      // right border
      if (xofs + len >= screenWIDTH)
      {
        len -= (xofs + len) - screenWIDTH;
      }
      int ofs = (i + y) * PITCH + xofs;

      // note that len may be 0 at this point, 
      // and no pixels get drawn!
      for (j = 0; j < len; j++)
        ((unsigned int*)gScreen->pixels)[ofs + j] = c;
    }
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void drawtile(int x, int y, int tile)
{

// Lock surface if needed
 if (SDL_MUSTLOCK(gTiles))
    if (SDL_LockSurface(gTiles) < 0) 
      return;

 if ((y + TILESIZE) >= 0 && (y  < screenHEIGHT))
   if ((x + TILESIZE) >= 0 && (x  < screenWIDTH)){
    int startx = x;
    int starty = y;
    int srcx = 0;
    int srcy = tile * TILESIZE;
    int tamax = TILESIZE;
    int tamay = TILESIZE;
    if ((x+TILESIZE) > screenWIDTH)
    	tamax = tamax - ((x+TILESIZE) - screenWIDTH);
    if ((y+TILESIZE) > screenHEIGHT)
        tamay = tamay - ((y+TILESIZE) - screenHEIGHT);
    if (x<0){
        startx = 0;
        srcx = -x;
        tamax = tamax + x;
    }
    if (y <0){
        starty = 0;
        srcy = srcy - y;
        tamay = tamay + y;
    }
    
    SDL_Rect sourceRect;
	sourceRect.x = srcx;
	sourceRect.y = srcy;
	sourceRect.w = tamax;
	sourceRect.h = tamay;

	
	SDL_Rect targetRect;
	targetRect.x = startx;
	targetRect.y = starty;
	targetRect.w = tamax;
	targetRect.h = tamay;
	
	SDL_BlitSurface(gTiles, &sourceRect, gScreen, &targetRect);
   }

  // Unlock if needed
    if (SDL_MUSTLOCK(gTiles)) 
        SDL_UnlockSurface(gTiles);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void ball_sprite(const char *filename, int sw, int sh)
{
	Uint32 pink;
	SDL_Surface *tmp = SDL_LoadBMP(filename);
	if(!tmp)
	{
		panic("Could not load sprite ball");
	}

	if(!sw)
		sw = tmp->w;
	if(!sh)
		sh = tmp->h;

	SDL_SetAlpha(tmp, SDL_SRCALPHA, 190);
	pink = SDL_MapRGB(tmp->format, 255, 0, 255);
	SDL_SetColorKey(tmp, SDL_SRCCOLORKEY, pink);
	gBall = SDL_DisplayFormatAlpha(tmp);
	if(!gBall)
			{
			panic("Could not convert alpha");
			}
	SDL_FreeSurface(tmp);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void draw_ball(int x, int y) {

// Lock surface if needed
 if (SDL_MUSTLOCK(gBall))
    if (SDL_LockSurface(gBall) < 0) 
      return;

	SDL_Rect sourceRect;
	sourceRect.x = 0;
	sourceRect.y = 0;
	sourceRect.w = gBall->w;
	sourceRect.h = gBall->h;
	
	SDL_Rect targetRect;
	targetRect.x = x;
	targetRect.y = y;
	targetRect.w = gBall->w;
	targetRect.h = gBall->h;

	SDL_BlitSurface(gBall, &sourceRect, gScreen, &targetRect);	
	
	// Unlock if needed
	 if (SDL_MUSTLOCK(gBall)) 
	        SDL_UnlockSurface(gBall);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void renderPixel( int x, int y, Uint8 R, Uint8 G, Uint8 B )
{
    Uint32 color = SDL_MapRGB( gScreen->format, R, G, B );
    
            Uint32 *bufp;
            bufp = (Uint32 *)gScreen->pixels + y*gScreen->pitch/4 + x;
            *bufp = color;

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void initStars( void )
{
    for( int i = 0; i < TOTAL_STARS; i++ )
    {
        stars[i].x = rand()%screenWIDTH;
        stars[i].y = rand()%screenHEIGHT;
        stars[i].velocity = 1 + rand()%16;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void scrollStars( void )
{
        // Scroll stars to the left
        for( int i = 0; i< TOTAL_STARS; i++ )
        {
            // Move the star
            stars[i].x -= stars[i].velocity;

            // If the star falls off the screen's edge, wrap it around
            if( stars[i].x <= 0 )
                stars[i].x = screenWIDTH;
        }
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void renderStars( void )
{
    SDL_FillRect( gScreen, NULL, SDL_MapRGB( gScreen->format, 0, 0, 0));

    if( SDL_MUSTLOCK( gScreen ) )
    {
        if( SDL_LockSurface(gScreen) < 0 )
            return;
    }

    for( int i = 0; i < TOTAL_STARS; i++ )
        renderPixel( stars[i].x, stars[i].y, 255, 255, 255 );


    if( SDL_MUSTLOCK( gScreen ) )
        SDL_UnlockSurface( gScreen );
}
