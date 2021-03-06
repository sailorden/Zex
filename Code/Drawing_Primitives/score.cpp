// Zex - 2D Space Trading Game with 3D graphics
// Copyright (C) 1998-2005  Stuart Ball and Robert Probin
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Name:   		    Score.c
// Description:		This routine, reads in an string (the score)
//					and then spats at the location specified
//
// Date Started:	31/19/98
// Author(s):		ID
//
// Copyright(c) Lightsoft Software

#include "options.h"		// should always be first include
#include "SDL.h"
#include "LS_Standard.h"


//oldmac//#include <ToolUtils.h>
//oldmac//#include <resources.h>
//#include <stdio.h>
#include "Score.h"
#include "Error_handler.h"

#include "zex_misc.h"

#define CHAR_OFFSET 96  // in Pixels to the next character
#define CHAR_WIDTH 8
#define CHAR_HEIGHT 12



void Draw_to_Double_Buffer_Score(char *char_ptr, int x_pos, int y_pos);

static Ptr double_buffer;
static int screen_width;
Handle score_ptr;


void InitScore()
 {
 score_ptr = (Handle) LS_GetResource('ZRAW',136);  //Get the Handle to the Resource
 if (score_ptr==0) report_error_c("Resource missing: ZRAW 136","",4);
 LS_HLock(score_ptr);
 }


//-----------------------------------------------------------------
//-----------------------------------------------------------------


void Draw_to_Double_Buffer_Score(char *char_ptr, int x_pos, int y_pos)
  {
extern Ptr screenaddr;
extern int screenwidth;
char *db_ptr;
int a,b;


double_buffer=screenaddr;
screen_width=screenwidth;

db_ptr=(char *) (double_buffer+x_pos+(y_pos*screen_width));

  for(b=0; b < CHAR_HEIGHT; b++)
  {
    for(a=0; a < CHAR_WIDTH; a++)
     {
     *db_ptr = *char_ptr;
     char_ptr++;
     db_ptr++;
     }
  char_ptr-=CHAR_WIDTH;
  db_ptr-=CHAR_WIDTH;
  char_ptr+=CHAR_WIDTH;
  db_ptr+=screen_width;
  }

  }

void SplatScore(char splat_str[30], int x_pos, int y_pos)
 {
  int i,my_char,counter;
  char *char_ptr;
//  char temp_str[20];
  char new_str[20];

  int char_pos;
//  Rect my_rect;


  char_pos=x_pos;
  i=0;
  counter = 0;

  my_char = splat_str[i];


  while(my_char)
   {
    if ((my_char >= '0') && (my_char <='9'))
    {
     counter++;
    }
     i++;
     my_char = splat_str[i];
   }


 counter  = 9 - counter;

  for (i= 0; i < counter; i++)
    {
     new_str[i] = '0';
    }


  for (i= counter; i < 9; i++)
    {
     new_str[i] = splat_str[i- (counter-1)];
     }

  new_str[i] = '\0';


  i = 0;
  my_char = new_str[i];


  while(my_char)
   {

    if ((my_char >= '0') && (my_char <='9'))
    {
     char_ptr =   (CHAR_OFFSET * (my_char - 48)) + *score_ptr;
     Draw_to_Double_Buffer_Score(char_ptr,char_pos,y_pos);
    }



   char_pos+= CHAR_WIDTH;
   i++;
   my_char=new_str[i];
   }


 }

