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

// *************************************MODULE HEADER*********************************
// * FILENAME           : Untitled 1
// * MODULE TITLE       : Untitled 1
// * PROJECT            : Zex
// * DATE STARTED       : 19:22:04 on 15th September 1999
// * FIRST AUTHOR       : Stuart Ball
// *
// * COPYRIGHT (c)        Lightsoft
// *
// ***********************************************************************************
// * HISTORY:
// * Date Initial         Descrip.
// *
// *
// *
// ***********************************************************************************


// ***********************************************************************************
// * INCLUDED FILES
// *
#include "options.h"		// should always be first include
#include "SDL.h"
#include "LS_Standard.h"

//oldmac//#include <NumberFormatting.h>
#include "ship_control.h"
#include "3d_structures_def.h"
#include	"load_objects.h"
#include	"objects_controls.h"
#include "object_control_top.h"
#include "explode.h"
#include "sound.h"
#include "fire_weapon.h"
#include "monster1.h"
#include "aim.h"
#include	"zex_misc.h"
#include "dlp.h"
#include "input_control.h"
#include "instrum_int.h"
#include "buy_int.h"
#include "Damage.h"
#include	"drawing.h"
#include "pictures.h"
#include "game_pause.h"
#include "Error_handler.h"
#include "compile_zmap.h"
#include "level_control.h"
#include "weapons_damage.h"
#include "camera.h"
#include "game_defs.h"
#include "obj_control_misc_routines.h"
#include "light_control.h"
#include "txtsplat.h"
#include "handle_level.h"
#include "motherships.h"
#include "cargo.h"
#include "command_and_control.h"
#include	"splat_control.h"
#include "wingman.h"
#include "dlp.h"
#include "monster_control_misc.h"
// ***********************************************************************************
// * CONSTANTS 
// *

#define DX 200
#define DY 100

// ***********************************************************************************
// * TYPE DEFINITIONS
// *



// ***********************************************************************************
// * VARIABLE DECLARATIONS 
// *

//we manage upto 4 wingmen
int wingmen_under_command[4];	//-1 if empty else ocb slot #
int command_slot_dialog;
// ***********************************************************************************
// * INTERNAL FUNCTION PROTOTYPES
// *
void render_frame(int width, int height);
void handle_command(LSRAW* cc_pict, LSRAW* command_dialog_pict);
void print_available_info(int x, int y);
void fill_rank (int rank, char * string);
void copystr (char *to, const char*from);
int process_cc_mouse();
int make_wingman(int slot, int from_where);
int check_under_command(int slot);
void print_current_info(int x, int y);
void release_wingman(int slot);

// +----------------------------------------------------------------ROUTINE HEADER----
// | TITLE              : 
// | AUTHOR(s)          :
// | DATE STARTED       : 
// +----------------------------------------------------------------------------------
// | DESCRIPTION:  
// | 
// | 
// |
// |
// +----------------------------------------------------------------------------------
// | PARAM IN   : none
// |
// | PARAM OUT  : none
// |
// | ASSUMPTIONS:
// | LIMITATIONS:
// +----------------------------------------------------------------------------------
// | HISTORY:
// | Date Initial                Descrip.
// |
// |
// +----------------------------------------------------------------------------------
// | ADDITIONAL NOTES:
// | 

extern int Use_Mode320;
extern int Mode320,game_running;
extern int gCommandControl;

void init_wingmen_under_command()
{
int i;
for (i=0;i<4;i++) wingmen_under_command[i]=-1;
command_slot_dialog=-1;
}

void do_command_and_control()
{
Handle cc_H;
LSRAW *cc_pict;
Handle command_dialog_H;
LSRAW *command_dialog_pict;

_3D light_normal_save;
extern int freeze;
extern _3D light_normal;
int quit;

command_slot_dialog=-1;

   cc_H=GetZexPicture ('RCZ ',3114);	//Command&Control pilot list
	LS_HLock(cc_H);
	cc_pict = (LSRAW*)*cc_H;

   command_dialog_H=GetZexPicture ('RCZ ',3115);	//Command&Control dialog     
	LS_HLock(command_dialog_H);
	command_dialog_pict = (LSRAW*)*command_dialog_H;

    freeze=1;	//in 3d_top
    light_normal_save=light_normal;
	light_normal.z=1.3;	//dim the scene


LS_ShowCursor();
handle_command(cc_pict,command_dialog_pict);

LS_HideCursor();


    LS_DisposeHandle(cc_H);
    LS_DisposeHandle(command_dialog_H);
    light_normal=light_normal_save;	//restore the lighting
     freeze=0;	//unfreeze engine

}


int done_available_list=0;
int available_slots[5];

extern	DynObjectsFixedSize *ocb_ptr;
extern unsigned int last_pause_frame;
extern int show_fps;

void handle_command(LSRAW* cc_pict, LSRAW* command_dialog_pict)
{
int quit;
int prefs_x,prefs_y;
int old_mode;
int i;
int available_slots_index;
unsigned int time_entered;


time_entered=LS_TickCount60();	//for K key check - wait 20 ticks


//build list of available pilots
if (done_available_list==0)
     {

	available_slots_index=0;   
	for (i=0;i<5;i++)   available_slots[i]=-1;
	
//scan slots looking for available pilots
      for (i=1;i<MAX_NUMBER_OF_DYN_OBJECTS-1;i++)
       {
          if ((*ocb_ptr).object_list[i].in_use==1)
             {
              if ((*ocb_ptr).object_list[i].object_category==HUMAN
                   && (*ocb_ptr).object_list[i].craft_type==TYPE_FALCON
                   && check_under_command(i)==0)
                 {
                 available_slots[available_slots_index]=i;
                 available_slots_index++;
                 if (available_slots_index>4) break;
                 }
             }
       }

     done_available_list=1;
     }
     
Mode320=0;

quit=0;
while(quit==0)
{

 if (command_slot_dialog==-1)	//show selector box
 {
 	 //Get_back_buffer();
     render_frame(640,390);	//in this file - calls all the needed 3d stuff
 
       prefs_x=DX; prefs_y=DY;
  	  print_crunched_translucent_pict(cc_pict,prefs_x,prefs_y);
  	  print_available_info(prefs_x,prefs_y);
  	  print_current_info (prefs_x,prefs_y);
 //    run_camera_control();;
 //     do_object_control();
 
     Show_it();	//splat 3d portion of window
     
 
     render_frame(640,390);	//in this file - calls all the needed 3d stuff
 
  	  print_crunched_translucent_pict(cc_pict,prefs_x,prefs_y);
  	  print_available_info(prefs_x,prefs_y);
  	  print_current_info (prefs_x,prefs_y);
 //     run_camera_control();;
 //     do_object_control();
 	 
    Show_it();	 //splat 3d portion of wind
 //check for keyboard input
    poll_input_device();
    if(gCommandControl==1 && LS_TickCount60()>time_entered+20) //gCommandControl is ISp input
    {
     quit=1;
     gCommandControl=0;
     last_pause_frame=LS_TickCount60()+20;
    }
 //check for mouse clicks
    quit|=process_cc_mouse();
 
 
 }	 //end of show_command_dialog_for_callsign==0
 
}	//end of while quit
if (Use_Mode320==1) Mode320=1;

done_available_list=0;
}




void run_control_dialog()
 {
   OSType ob_type; 
   Byte *bp;
   char the_string[5];
   int prefs_x,prefs_y;
   extern LSRAW *order_dialog;
   
//#define DX 0
//#define DY 0
       prefs_x=0; prefs_y=0;
  	  print_crunched_translucent_pict(order_dialog,prefs_x,prefs_y);
  	  //draw text
  	      SplatText_Small_Transparent("COMMANDS FOR ",prefs_x+10,prefs_y+3,246);
	   ob_type =(*ocb_ptr).object_list[command_slot_dialog].callsign;
	   bp = (Byte *)&ob_type;
       the_string[0]=bp[0];
       the_string[1]=bp[1];
       the_string[2]=bp[2];
       the_string[3]=bp[3];
       the_string[4]=0;
  	      SplatText_Small_Transparent(the_string,prefs_x+72,prefs_y+3,246);
          prefs_y+=13;
  	      SplatText_Small_Transparent("1 REPORT STATUS",prefs_x+10,prefs_y,246);
  	      SplatText_Small_Transparent("2 HOLD POSITION",prefs_x+100,prefs_y,246);
  	      SplatText_Small_Transparent("3 FOLLOW ME",prefs_x+200,prefs_y,246);
  	      SplatText_Small_Transparent("4 STAND DOWN",prefs_x+300,prefs_y,246);
  	      SplatText_Small_Transparent("5 CANCEL COMMAND",prefs_x+400,prefs_y,246);

//check for command keys - 1 to 9
    
//check for keyboard input
   if (IsKeyPressed(SDLK_1))	//1 key
     {
     //report status
     (*ocb_ptr).object_list[command_slot_dialog].command=SEND_STATUS;
     command_slot_dialog=-1;
     
     }
     else
    if (IsKeyPressed(SDLK_2))	//2 key
     {
     //report status
     (*ocb_ptr).object_list[command_slot_dialog].command=HOLD_POSITION;
     command_slot_dialog=-1;
     }
     else
    if (IsKeyPressed(SDLK_3))	//3 key
     {
     //report status
     (*ocb_ptr).object_list[command_slot_dialog].command=FOLLOW_ME;
     command_slot_dialog=-1;
     }
     else
    if (IsKeyPressed(SDLK_4))	//4 key
     {
     //report status
     (*ocb_ptr).object_list[command_slot_dialog].command=STAND_DOWN;

     release_wingman(command_slot_dialog);	//remove from our list of pilots under command
     command_slot_dialog=-1;
     }
     else
    if (IsKeyPressed(SDLK_5))	//5 key - cancel
     {
     command_slot_dialog=-1;
     }

     else
    if (IsKeyPressed(SDLK_9))	//9 key - show fps
     {
      show_fps=show_fps^1;
     command_slot_dialog=-1;
     }
    
     
 }




//return 1 to quit, else 0
int process_cc_mouse()
{
Point mouse_xy;
int topline,i;
extern UInt32 gAbort;

if (gAbort) 
{
	gAbort=0;	//eat this else we'll get the end game dialog
	return  1;	//return quit if abort key pressed
}
if (LS_Left_Button()==0)
return 0;
//button is down
     ZexGetMouse (&mouse_xy);
if (mouse_xy.h<DX || mouse_xy.h>DX+260 || mouse_xy.v<DY || mouse_xy.v>DY+220) 
    {
      play_zsound_always(low_beep, sound_high_pri, c1_chan1, sound_vol_7);
      return 1;	//click outside dialog quits C&C dialog
    }
else	//this is button down
{
//in top half of box?
topline=-1;
for (i=0;i<5;i++)
   {
   if (mouse_xy.v>DY+20+(i*20) && mouse_xy.v<DY+39+(i*20))  { topline=i; break; }
   }
   
if (topline!=-1)
   {
   //yes topline
      if(available_slots[topline]!=-1)
          {
          //a ship was available, copy to wingmen slots if one is free
                if (make_wingman(available_slots[topline],topline)==-1) return 1;
                

          }
   }

//try bottom of box, which is pilots under command
topline=-1;
for (i=0;i<4;i++)
   {
   if (mouse_xy.v>DY+140+(i*20) && mouse_xy.v<DY+159+(i*20))  { topline=i; break; }
   }
  
if (topline!=-1)
   {
   //yes topline
      if(wingmen_under_command[topline]!=-1)
          {
           //a wingman was found, store its ocb in 
                command_slot_dialog=wingmen_under_command[topline];
                play_zsound_always(low_beep, sound_high_pri, c1_chan1, sound_vol_7);
                     gCommandControl=0;
                     return 1;	//quit

          }
   }
   
}	//end of button down
return 0;
}


//returns 1 if OCB is already under command
int check_under_command(int slot)
{
int i;
//find an empt slot
for (i=0;i<4;i++)
 {
  if (wingmen_under_command[i]==slot) return 1;
 }
 
return 0; 

}

int make_wingman(int slot, int from_where)
{
int i;
extern int gRank;
//check we are a higher rank than the wingman
if (gRank>(*ocb_ptr).object_list[slot].rank)
{
//find an empt slot
for (i=0;i<4;i++)
 {
  if (wingmen_under_command[i]==-1)
     {
     wingmen_under_command[i]=slot;
     (*ocb_ptr).object_list[slot].controller=WINGMAN_FOR_GAME;
     (*ocb_ptr).object_list[slot].control_use_9=0;	 //init wingman
     (*ocb_ptr).object_list[slot].command=FOLLOW_ME;

     available_slots[from_where]=-1;
     return -1;			// got wingman, return and kill dialog
     }
 }
return 0;		// keep wingman list there
}	//end of rank check
else
{
Handle small_dialog_H;
LSRAW *small_dialog_pict;
int prefs_x,prefs_y;

   small_dialog_H=GetZexPicture ('RCZ ',3117);	//Command&Control dialog     
	LS_HLock(small_dialog_H);
	small_dialog_pict = (LSRAW*)*small_dialog_H;

   while (LS_Left_Button()==1)  { SDL_Delay(10); Show_it(); } // give computer back time and allow mouse to move (if on screen)
   play_zsound_always(low_beep, sound_high_pri, c1_chan1, sound_vol_7);
//show no can do box
     render_frame(640,390);	//in this file - calls all the needed 3d stuff
 
      prefs_x=256; prefs_y=100;
  	  print_crunched_translucent_pict(small_dialog_pict,prefs_x,prefs_y);
	   SplatText_Transparent_Fade ("You can't command",prefs_x+7,prefs_y+20,246);
	   SplatText_Transparent_Fade ("that pilot because",prefs_x+7,prefs_y+32,246);
	   SplatText_Transparent_Fade ("your rank is too low.",prefs_x+7,prefs_y+44,246);

 
     Show_it();	//splat 3d portion of window
   LS_DisposeHandle(small_dialog_H);
   while (LS_Left_Button()==0)  { SDL_Delay(10); Show_it(); } // give computer back time and allow mouse to move (if on screen)

 return -1;		// didn't make a wingman but clear down dialog anyway
}
}
#define colour 246

void print_available_info(int x, int y)
{
int i;
char the_string[256];
  OSType ob_type; 
  Byte *bp;

	for (i=0;i<5;i++)   
	 {
	 if (available_slots[i]!=-1)
	   {
	   //draw ID
	   ob_type =(*ocb_ptr).object_list[available_slots[i]].callsign;
	   bp = (Byte *)&ob_type;
       the_string[0]=bp[0];
       the_string[1]=bp[1];
       the_string[2]=bp[2];
       the_string[3]=bp[3];
       //draw craft type
       the_string[4]=' ';
       the_string[5]='F';
       the_string[6]='a';
       the_string[7]='l';
       the_string[8]='c';
       the_string[9]='o';
       the_string[10]='n';
       the_string[11]=0;
	   SplatText_Transparent_Fade (the_string,x+10,y+24,colour);

       fill_rank((*ocb_ptr).object_list[available_slots[i]].rank,&the_string[0]);
	   SplatText_Transparent_Fade (the_string,x+150,y+24,colour);
       
       }
//	else
//	SplatText_Transparent_Fade ("None",x+10,y+24,246);
      y+=20;
     }
//RGBForeColor(&rgb
//TextFace(0);
//TextSize(12);
// MoveTo(x+10,y+30);
// DrawString("Hello");
}




void print_current_info(int x, int y)
{
int i;
char the_string[256];
  OSType ob_type; 
  Byte *bp;

	for (i=0;i<4;i++)   
	 {
	 if (wingmen_under_command[i]!=-1)
	  {
	   if ((*ocb_ptr).object_list[wingmen_under_command[i]].in_use==0 ||
	   (*ocb_ptr).object_list[wingmen_under_command[i]].controller!=WINGMAN_FOR_GAME)
	   {
	    wingmen_under_command[i]=-1;
	   }
	   else
	   {
	    //draw ID
	    ob_type =(*ocb_ptr).object_list[wingmen_under_command[i]].callsign;
	    bp = (Byte *)&ob_type;
        the_string[0]=bp[0];
        the_string[1]=bp[1];
        the_string[2]=bp[2];
        the_string[3]=bp[3];
        //draw craft type
        the_string[4]=' ';
        the_string[5]='F';
        the_string[6]='a';
        the_string[7]='l';
        the_string[8]='c';
        the_string[9]='o';
        the_string[10]='n';
        the_string[11]=0;
	    SplatText_Transparent_Fade (the_string,x+10,y+144,colour);

        fill_rank((*ocb_ptr).object_list[wingmen_under_command[i]].rank,&the_string[0]);
	    SplatText_Transparent_Fade (the_string,x+150,y+144,colour);
        }
       }
//	else
//	SplatText_Transparent_Fade ("None",x+10,y+24,246);
      y+=20;
     }
//RGBForeColor(&rgb
//TextFace(0);
//TextSize(12);
// MoveTo(x+10,y+30);
// DrawString("Hello");
}





void fill_rank (int rank, char * string)
{

             if (rank==ENS) 
             {
             copystr (string,"Ensign");
             }
             else
             if (rank==LT) 
             {
             copystr(string,"Lieutenant");
             }
             else
             if (rank==LTCR) 
             {
             copystr(string,"LT Commander");
             }
             else
             if (rank==CDR) 
             {
             copystr(string,"Commander");
             }
             else
             if (rank==CAPT) 
             {
             copystr(string,"Captain");
             }
             else
             if (rank==RADM) 
             {
             copystr(string,"Rear Admiral");
             }
             else
             if (rank==VADM) 
             {
             copystr(string,"Vice Admiral");
             }
             else
             if (rank==ADM) 
             {
             copystr(string,"Admiral");
             }
             else
             if (rank==FADM) 
             {
             copystr(string,"Fleet Admiral");
             }
}


//run through OCB's and change any wingmen for game to npc_human
void 
stand_down_wingmen()
{
int i;

      for (i=1;i<MAX_NUMBER_OF_DYN_OBJECTS-1;i++)
       {
          if ((*ocb_ptr).object_list[i].in_use==1)
             {
              if ((*ocb_ptr).object_list[i].controller==WINGMAN_FOR_GAME)
                 {
                 (*ocb_ptr).object_list[i].controller=NPC_CONTROL;	//revert to normal control
                      (*ocb_ptr).object_list[i].current_mode=1;	//back to normal mode
                 }
             }
       }
//now clear the under_command array
	for (i=0;i<4;i++)   
	 wingmen_under_command[i]=-1;

}

void release_wingman(int slot)
{
int i;
 for (i=0;i<4;i++)
 {
 if (wingmen_under_command[i]==slot) wingmen_under_command[i]=-1;
 
 }
}


void copystr (char *to, const char*from)
{
char temp;
temp=-1;
 while (temp!=0)
  {
  temp=*from;
  *to=temp;
  to++;
  from++;
  }
}


