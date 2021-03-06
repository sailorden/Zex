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

//map_control.c

#include "options.h"		// should always be first include
#include "SDL.h"
#include "LS_Standard.h"

#include <stdlib.h>

//oldmac//#include <NumberFormatting.h>
#include "3d_structures_def.h"
#include "load_objects.h"
#include "map_control.h"
#include "Galaxy.h"
#include "object_misc.h"
#include "compile_zmap.h"
#include "Error_handler.h"
#include "zex_misc.h"
#include "monster_control_misc.h"
#include "level_control.h"
#include "ship_control.h"
#include	"camera.h"
#include "explode.h"
#include "object_control_top.h"
#include "obj_control_misc_routines.h"
#include "objects_controls.h"	//contains monster defs
#include "motherships.h"
#include "game_defs.h"
#include "dlp.h"
#include "sound.h"
#include "texture_management.h"
#include "Planet_Generator.h"
#include	"drawing.h"
#include "pictures.h"
#include "game_pause.h"
#include "Batcom.h"
#include "load_complex_object.h"
#include "OGL.h"
#include "game_script.h"
#include "instrum_int.h"
#include "Damage.h"
#include "file_io.h"
#include "input_control.h"
#include "CPanel.h"
#include	"splat_control.h"
void load_mothership(mothership_block * the_block);


void unload_ocbs(int quad, int section);
//void load_ocbs(int quad, int section, int section_x, int section_y);
void load_planet(float x, float y, int seed, short galactic_id,int is_inhabited);
int find_galactic_id_in_ocbs(short the_id);
void load_special(float x, float y, int code, short galactic_id);
void load_scenary(float x, float y, int seed, short galactic_id);
void load_warphole(float x, float y, short galactic_id);
void run_warphole();
void kill_all_things();
void load_star(int static_slot);
void load_galaxy(int static_slot);
void load_ring(int static_slot);
void load_shooter_1(int static_slot);
void load_sphere(int static_slot, int rot_z);
void load_transporters(int quad, int section);
int current_quad;	//for game use, from run_map_control
int warphole_slot;

int old_quad,old_section;
int ocbs_holding_which_section;
int ocbs_holding_which_quad;
int next_frame_run=0;
int map_update_count=76;



static int warphole_core_slot,warphole_trunk;

extern int training_mission,tutorial;
extern int real_frame_counter;
extern	DynObjectsFixedSize *ocb_ptr;
extern int level_has_control,disable_camera_control;

extern int can_gen_dust;
extern int done_monsters;	//used to control when we set off monsters
extern int LFrame_counter,total_frame_counter;	//timer
extern int next_ast_time;
extern int interval;	//how long before we gen a monster
extern	DynObjectsFixedSize *ocb_ptr;
extern	float SinTable[360], CosTable[360];	//fancy see this here!!!
extern int var_a,tractor;
extern int wingmen_guarding_what_object;	//the object wingmen are guarding
extern int gRadarRange,thrust;
extern int    global_open_doors;
extern int dynamic_object_index;
extern float camera_zoom;
extern _3D camera_pos;
extern int camera_rotation_z;	//obviously the camera rot
extern int camera_rotation_x;
extern int camera_rotation_y;
extern int can_do_next_level;
extern int in_wormhole;
extern int in_ramparian_space;
extern int mothership_core_slot;
extern int wingmen_guarding_what_object,game_running;	//the object wingmen are guarding

extern  int entered_hole;
extern unsigned int obc_warning_timer;
extern int gen_dust_timer;
extern int over_hole;
extern int vib;
extern int roty;
extern int shooter1_slot,shooter2_slot,star_slot,mon2_slot,ball1_slot,gala_slot,ring_slot;
extern int timer_5,timer_6,cy_delta;
extern int frame_count,cam_rot_d;
extern int star_rot,sphere_rot;
extern int rumble_volume;
extern int midi_note,inc_midi;
extern int end_scene;
static unsigned char bg_colour;
static float quad_x,quad_y;
extern prefs gzex_prefs;
extern int OGL_active;


static int done_warphole_message=0;	//unregistered warphole message if 0 (once per session)

#define BUBBLE_SIZE 1000000	//when we unload ocb's if an object is within this range
                                //it is not unloaded


//this routine forces the OCB's to be unloaded and reloaded,
//usually because a mothership has changed allegiance
//change will happen on next frame
void force_ocb_update()
{
old_section=-1;
next_frame_run=real_frame_counter;
}

void periodic_game_checks(void)
{
galactic_stats the_stats;
	//check if RMparians are going to surrender
	 get_galactic_stats(&the_stats);
	//if the ration of earth motherships to alien motherships is greater than 50:1 then surrender
	float earth_ratio=(float)the_stats.number_of_earth_mships/(float)the_stats.number_of_alien_mships;
	if ((earth_ratio>50.0) && (surrendered_flag==0))
	{
		surrendered_flag=1;	//this is a saved global
		script_timer1=1000;	//time till we show surrendered dialog
		script_state1=BETA_MESSAGE;
		handle_surrender();	//in game_script.cpp
	}


	//Check if we should do some re-population of RMparians
}

int return_promotion_points_for_rank(int rank)		// returns 0 for no more promotions
{
	int promotion_points=0;

	if (rank==ENS)	//ties up with draw_topbottom_lines and show_ratings
	{
		promotion_points = 300;	//these must match those in docked
	}
	else if (rank==LT)
	{
		promotion_points = 1500;
	}
	else if (rank==LTCR)
	{
		promotion_points = 6000;
	}
	else if (rank==CDR)
	{
		promotion_points = 18000;
	}
	else if (rank==CAPT)
	{
		promotion_points = 35000;
	}
	else if (rank==RADM)
	{
		promotion_points = 60000;
	}
	else if (rank==VADM)
	{
		promotion_points = 80000;
	}
	else if (rank==ADM)
	{
		promotion_points = 100000;
	}
	else
	{
		promotion_points = 0;
	}

	return promotion_points;
}


void run_map_control()	//called every frame
{
int ship_x,ship_y;
int quad,promoted;
int section_x,section_y,section;
extern int promotion_points,gRank;

update_script();
if (training_mission!=0 || tutorial!=0) return;	//only do for real!


 ship_x=static_cast<int>((*ocb_ptr).object_list[0].Oworldx);
 ship_y=static_cast<int>((*ocb_ptr).object_list[0].Oworldy);

if (warphole_slot!=-1)
{
run_warphole();
}

if (level_has_control) return;	//we're in the warphole

if (real_frame_counter>next_frame_run)
 {
 next_frame_run=real_frame_counter+map_update_count;


periodic_game_checks();


 promoted=0;

 int next_pp = return_promotion_points_for_rank(gRank);
 if(next_pp)
 {
	 if(promotion_points >= next_pp)
	 {
		 promoted=1;
	 }
 }


if (promoted==1)
{
      play_zsound_always(low_beep, sound_high_pri, c1_chan1, sound_vol_7);

   gRank++;
   promotion_points=0;
   add_to_text_display("OBC> Message from Command incoming. It's coded... Analysing...: ",0);
   add_to_text_display("OBC> FIELD PROMOTION!",0);

if (gRank==LT)
{
	add_to_text_display("OBC> WELL DONE LIEUTENANT!",0);
	add_to_text_display("OBC> YOU CAN NOW COMMAND OTHER SHIPS.",0);
}
else
if (gRank==LTCR)    add_to_text_display("OBC> WELL DONE LIEUTENANT COMMANDER!",0);
else
if (gRank==CDR)    add_to_text_display("OBC> WELL DONE COMMANDER!",0);
else
if (gRank==CAPT)    add_to_text_display("OBC> WELL DONE CAPTAIN!",0);
else
if (gRank==RADM)    add_to_text_display("OBC> WELL DONE REAR ADMIRAL!",0);
else
if (gRank==VADM)    add_to_text_display("OBC> WELL DONE VICE ADMIRAL!",0);
else
if (gRank==ADM)    add_to_text_display("OBC> WELL DONE ADMIRAL!",0);
else
if (gRank==FADM)    add_to_text_display("OBC> WELL DONE FLEET ADMIRAL!",0);

}
//calc if promote


 //calc quad
 	 if (ship_x<GALAXY_WIDTH/2)
	   {
	   //quad 0 or 2
	    if (ship_y<GALAXY_H/2)
	      quad=2;
	    else
	      quad=0;
	   }
	else
	   {
	   //quad 1 or 3
	    if (ship_y<GALAXY_H/2)
	      quad=3;
	    else
	      quad=1;
	   }


if (old_quad!=quad && old_quad!=-1)
{	//the idea here is to stop the player just wandering into
return;	//a new quadrant
}

 //calculate section x and section y (I do it 2d rather than 1d for ease of visualisation
 //normalise x fort quads 1 and 3
 if (quad==1 || quad==3) ship_x-=GALAXY_WIDTH/2;
 section_x=ship_x/SECTION_SIZE;	//0-9 hopefully

 //normalise y for quad 0 and
 if (quad==0 || quad==1) ship_y-=GALAXY_WIDTH/2;
 ship_y=GALAXY_WIDTH/2-ship_y;	//invert so min is at top (top is GALAXY_WIDTH)
 section_y=ship_y/SECTION_SIZE;

     section=section_y*NUMBER_OF_SECTIONS_IN_A_QUAD_Y+section_x;

 if (old_section!=section)
  {
	old_quad=quad;	//when we do a load, we load the current quad
	current_quad=quad;	//for rest of system

  	//do the load/unload
  	if (ocbs_holding_which_section>=0)
	{
  	   unload_ocbs(quad,ocbs_holding_which_section);
    }
		    //load
        load_ocbs(quad,section);	//simple huh!
        old_section=section;
        ocbs_holding_which_section=section;
  }
 }
}

void calc_current_quad()
{
int ship_x,ship_y;

 ship_x=static_cast<int>((*ocb_ptr).object_list[0].Oworldx);
 ship_y=static_cast<int>((*ocb_ptr).object_list[0].Oworldy); 	if (ship_x<GALAXY_WIDTH/2)
	   {
	   //quad 0 or 2
	    if (ship_y<GALAXY_H/2)
	      current_quad=2;
	    else
	      current_quad=0;
	   }
	else
	   {
	   //quad 1 or 3
	    if (ship_y<GALAXY_H/2)
	      current_quad=3;
	    else
	      current_quad=1;
	   }
}

void unload_ocbs(int quad, int section)
{
int i,dist;
//kill everything bar zex
for (i=1;i<MAX_NUMBER_OF_DYN_OBJECTS-1;i++)
   {
    if ((*ocb_ptr).object_list[i].in_use==1)
    {
		//if it is close enough do not kill it
		//if object is a wingman, do not kill it
		//irrespective of the above two, if it's a mothership, kill it
		if ((*ocb_ptr).object_list[i].controller!=WINGMAN_FOR_GAME)	//dont kill wingmen
		{
			dist=get_distance_to_from(i,0);
			if (dist>BUBBLE_SIZE)	//if the item is too far away, kill it.
			{
				kill(i);
			}
			else	//item is fairly close
			{
				if (is_a_mothership(i)==1)	//we have to kill motherships in case of a surrender
				{
					kill(i);
				}

			}
		}	//end of not a wingman
	}

   }
}

void load_ocbs(int quad, int section)
{
int mship_index,i;
int section_x,section_y;
float stop, sleft, sbottom, sright;
float px, py;
int number_of_scenary_loaded;
unsigned char number_string[12];
extern galactic_map *GM_ptr;	//ptr to the galactic map
//this routine loads planets and motherships out of a section into the ocb's
//motherships
old_section=section;	//prevent double loads on load game (Hard bug!)
ocbs_holding_which_section=section; //prevent another hard bug when a mothership surrenders after a load game
									//unload wouldn't because ocbs_holding_which_section was -1
number_of_scenary_loaded=0;

 mship_index=section*NUMBER_OF_MOTHERSHIPS_IN_A_SECTION;
//there are always two motherships in a section
 for (i=0;i<NUMBER_OF_MOTHERSHIPS_IN_A_SECTION;i++)
 {
  if (find_galactic_id_in_ocbs(GM_ptr->the_quadrant[quad].quadrant_motherships[mship_index+i].galactic_id)==0)
  load_mothership(&GM_ptr->the_quadrant[quad].quadrant_motherships[mship_index+i]); //load it if not present
 }
 load_motherships_into_nav();	//mothership vectors for radar display (200299)

//planets

// planet_index=section*NUMBER_OF_PLANETS_IN_A_SECTION;
//there are always two motherships in a section
// for (i=0;i<NUMBER_OF_PLANETS_IN_A_SECTION;i++)
// {
//calc section x and y
//start of quadrant x/y
  if (quad==0 || quad==2) section_x=0;
  else section_x=NUMBER_OF_SECTIONS_IN_A_QUAD_X*SECTION_SIZE;

  if (quad==0 || quad==1) section_y=NUMBER_OF_SECTIONS_IN_A_QUAD_Y*SECTION_SIZE*2;
  else
  section_y=NUMBER_OF_SECTIONS_IN_A_QUAD_Y*SECTION_SIZE;
//add in section offset
  section_x+=(section%10)*SECTION_SIZE;
  section_y-=(section/10)*SECTION_SIZE;

  stop=section_y;
  sleft=section_x;
  sbottom=section_y-SECTION_SIZE;	//smaller
  sright=section_x+SECTION_SIZE;

//scan all planets looking for those that fall into this rect

  for (i=0;i<NUMBER_OF_PLANETS_PER_QUAD;i++)
   {
     px=GM_ptr->the_quadrant[quad].quadrant_planets[i].real_coords.x;
     py=GM_ptr->the_quadrant[quad].quadrant_planets[i].real_coords.y;
     if (px>sleft && px<sright && py<stop && py>sbottom)
     {
  if (find_galactic_id_in_ocbs(GM_ptr->the_quadrant[quad].quadrant_planets[i].galactic_id)==0)
       load_planet(px,py,GM_ptr->the_quadrant[quad].quadrant_planets[i].seed,
       GM_ptr->the_quadrant[quad].quadrant_planets[i].galactic_id,
       GM_ptr->the_quadrant[quad].quadrant_planets[i].inhabited);
     }
   }



//specials

//scan all specials looking for those that fall into this rect

  for (i=0;i<NUMBER_OF_SPECIALS_PER_QUAD;i++)
   {
     px=GM_ptr->the_quadrant[quad].quadrant_specials[i].real_coords.x;
     py=GM_ptr->the_quadrant[quad].quadrant_specials[i].real_coords.y;
     if (px>sleft && px<sright && py<stop && py>sbottom)
     {
       if (find_galactic_id_in_ocbs(GM_ptr->the_quadrant[quad].quadrant_specials[i].galactic_id)==0)
       load_special(px,py,GM_ptr->the_quadrant[quad].quadrant_specials[i].code,
       GM_ptr->the_quadrant[quad].quadrant_specials[i].galactic_id);
     }
   }

//Scenary -- Galaxies...
  for (i=0;i<NUMBER_OF_SCENARY_PER_QUAD;i++)
   {
     px=GM_ptr->the_quadrant[quad].quadrant_scenary[i].real_coords.x;
     py=GM_ptr->the_quadrant[quad].quadrant_scenary[i].real_coords.y;
     if (px>sleft && px<sright && py<stop && py>sbottom)
     {
       if (find_galactic_id_in_ocbs(GM_ptr->the_quadrant[quad].quadrant_scenary[i].galactic_id)==0)
       {
        load_scenary(px,py,GM_ptr->the_quadrant[quad].quadrant_scenary[i].seed,
        GM_ptr->the_quadrant[quad].quadrant_scenary[i].galactic_id);
       number_of_scenary_loaded++;
       }
     }
   }

//show how many scenaries were loaded
//   add_to_text_display("ZEX DEBUG> Number of scenary items loaded in this section =",0);
//    ZexNumToString(number_of_scenary_loaded,number_string);
//    number_string[number_string[0]+1]=0;	  //quick p to c hack
//    number_string[0]=' ';
//   add_to_text_display((char *) number_string,0);


//The warphole
     px=GM_ptr->the_quadrant[quad].the_warphole.real_coords.x;
     py=GM_ptr->the_quadrant[quad].the_warphole.real_coords.y;
     if (px>sleft && px<sright && py<stop && py>sbottom)	//warphole is in this miniblock
     {
      // if (find_galactic_id_in_ocbs(GM_ptr->the_quadrant[quad].the_warphole.galactic_id)==0)	//is wh already loaded in ocb's?
		{	//no
			load_warphole(px,py,GM_ptr->the_quadrant[quad].the_warphole.galactic_id);
		}
	}
	else	//no wh in this miniblock
	{
		warphole_slot=-1;	//tells system if a warphole is active
	}


//Load in some Alien transporters - this purely dynamic and doesn't come out of the map
load_transporters(quad, section);
}


void load_transporters(int quad, int section)
{
int section_x,section_y;
float stop, sleft, sbottom, sright;
float px, py;
float t_x, t_y;
int static_slot,dyn_slot;
load_dyn_object_pb the_params;

if (RangedRdm(0,10)>4) return;

//calc section x and y
//start of quadrant x/y
  if (quad==0 || quad==2) section_x=0;
  else section_x=NUMBER_OF_SECTIONS_IN_A_QUAD_X*SECTION_SIZE;

  if (quad==0 || quad==1) section_y=NUMBER_OF_SECTIONS_IN_A_QUAD_Y*SECTION_SIZE*2;
  else
  section_y=NUMBER_OF_SECTIONS_IN_A_QUAD_Y*SECTION_SIZE;
//add in section offset
  section_x+=(section%10)*SECTION_SIZE;
  section_y-=(section/10)*SECTION_SIZE;

  stop=section_y;
  sleft=section_x;
  sbottom=section_y-SECTION_SIZE;	//smaller
  sright=section_x+SECTION_SIZE;

 t_x=((sright-sleft)/2)+sleft;
 t_y=((stop-sbottom)/2)+sbottom;

        static_slot=get_static_object_slot('ATRA');	// Alien transporter
        if (static_slot==-1) report_error_c ("map_control:Couldn't find ATRA","",-5);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {
		clear_pb(&the_params);
		the_params.world_x=t_x;
		the_params.world_y=t_y;
		the_params.world_z=MONSTER_Z;
        the_params.has_roll=0;
        the_params.max_vel=60;
        the_params.min_vel=-60;
        the_params.aggressive=1;
        the_params.sub_controller=255;	// used to be -1;
        the_params.shields_class=5;
        the_params.controller_ref=NPC_CONTROL;
        the_params.shield_value=29890;
        the_params.mass=1000;
        the_params.no_trivial_rejection=1;
        the_params.object_category=baddie_object;
        the_params.cannon_cat=3;
        the_params.laser_cat=3;
		 the_params.missile_cat=3;
		 the_params.number_of_missiles=20;

		the_params.who_spawned_this_object=-1;
        the_params.obj_class=NPC_CLASS_FIGHTER;
		load_dyn_object(static_slot,dyn_slot,&the_params,-1);	//the object, the position (-1=next free)
        }


}



void load_warphole(float x, float y, short galactic_id)
{
int static_slot,dyn_slot;

register ZObject * current_object_ptr;
load_dyn_object_pb the_params;


        static_slot=get_static_object_slot('WH1Z');	// the center object
        if (static_slot==-1) report_error_c ("map_control:load_warphole couldn't find WH1Z","",-5);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {
          insert_continuous_sound(dyn_slot, rumble_loop,65);
          warphole_slot=dyn_slot;
          clear_pb(&the_params);
	  the_params.world_x=x;

	  the_params.world_y=y;
	  the_params.world_z=100;	//RangedRdm(2000,6000);
 	  the_params.Omovedx=0;
 	  the_params.Omovedy=0;
	  the_params.shield_value=31100;
	  the_params.mass=11000;


	  the_params.delta_rot_z=-1;
	  the_params.object_category=SCENARY;
	  the_params.controller_ref=0;

	  the_params.no_trivial_rejection=1;	//controlled when not in view
          the_params.who_spawned_this_object=-1;
	  load_dyn_object(static_slot,dyn_slot,&the_params,galactic_id);	//the object, the position (-1=next free)

        }


        static_slot=get_static_object_slot('WH2&');	// the center object
        if (static_slot==-1) report_error_c ("map_control:load_warphole couldn't find WH2&","",-5);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {

          clear_pb(&the_params);
	  the_params.world_x=x;

	  the_params.world_y=y;
	  the_params.world_z=100;	//RangedRdm(2000,6000);
 	  the_params.Omovedx=0;
 	  the_params.Omovedy=0;
	  the_params.shield_value=31100;
	  the_params.mass=11000;


	  the_params.delta_rot_z=-1;
	  the_params.object_category=SCENARY;
	  the_params.controller_ref=0;

	  the_params.no_trivial_rejection=1;	//controlled when not in view
          the_params.who_spawned_this_object=-1;
	  load_dyn_object(static_slot,dyn_slot,&the_params,galactic_id);	//the object, the position (-1=next free)

        }
 warphole_core_slot=find_dynamic_object_slot('WH1Z');
 warphole_trunk=find_dynamic_object_slot('WH2&');
 if (warphole_core_slot==-1) report_error_c("Load_warphole: Couldn't find warphole.","",-3);

//reset WH logic bits
   global_open_doors=1;
   entered_hole=0;
   obc_warning_timer=0;
   gen_dust_timer=0;
   over_hole=0;
   vib=30;
   roty=0;
   timer_5=0;
   cy_delta=10;
   frame_count=0;
   cam_rot_d=1;
   star_rot=0;
   bg_colour=255;
   timer_6=0;
   sphere_rot=0;
   rumble_volume=100;
   midi_note=42;
   inc_midi=7;
   end_scene=0;


}



//same as a planet, but we don't need temp storage, and we rot around x +-45 degrees
void load_scenary(float x, float y, int seed, short galactic_id)
{
int static_slot,dyn_slot;

register ZObject * current_object_ptr;
load_dyn_object_pb the_params;
dynamic_texture * my_tex_ptr;
_ZPoly *current_poly;
int number_of_polys_in_object,i;
int rot,err;

//calc section rect


//we use PNTZ, a dummy rectangle as our planet.
//we poke the address of our gen on the fly texture into all the polys
//[lanets live between 121 and 200 thousand Z

        if (seed%10<6)	//static object?
		{
			rot=0;

			//static dust cloud
			if (seed%8<2)
			{
				static_slot=get_static_object_slot('CLO1');	// the cloud object
				rot=RangedRdm(0,6)-3;
			}
			else
//			if (seed%8<4) //big green clouds - removed
//			{
//			static_slot=get_static_object_slot('CLO2');	// the cloud object
//			   rot=1;
//			}
//			else
			if (seed%8<6)
			{
			static_slot=get_static_object_slot('CLO3');	// star
			}
			else
			{
			static_slot=get_static_object_slot('CLO4');	// big transulent ball
			rot=RangedRdm(0,6)-3;
			}

			if (static_slot==-1) report_error_c ("map_control:load_scenary couldn't find CLO&","",-5);
			dyn_slot=find_vacant_dynamic_object_slot();
			if (dyn_slot!=-1)
			{
				clear_pb(&the_params);
				the_params.world_x=x;
				the_params.world_y=y;
				the_params.world_z=(seed%2000)+20000;	//Move clouds below zex always
				//	the_params.world_z=(ABS(seed/4)-1000);
				//          if (the_params.world_z<121000) the_params.world_z=121000;
				the_params.rot_z=seed%180;
				the_params.Omovedx=0;
				the_params.Omovedy=0;
				the_params.Omovedz=0;
				the_params.delta_rot_z=rot;
				#if OGL==1
				the_params.delta_rot_x=rot;
				#endif

				//the_params.rot_x=(seed%90)-45;	//+-45 (no longer rotate around x, just z.
				the_params.object_category=SCENARY;
				the_params.controller_ref=planet_rot_y;
				//the_params.rot_x=RangedRdm(3,8);

				the_params.no_trivial_rejection=0;	//controlled when not in view
				the_params.who_spawned_this_object=-1;
				load_dyn_object(static_slot,dyn_slot,&the_params,galactic_id);	//the object, the position (-1=next free)
				//now set up the motherships memory
				current_object_ptr=&ocb_ptr->object_list[dyn_slot];


				//galaxy_gen(seed, my_tex_ptr->tex_data);
				//poke the texture into the object - we assume all polys are covered (they are)
				number_of_polys_in_object=current_object_ptr->number_of_polys;
				current_poly=current_object_ptr->polys_ptr;
				for (i=0;i<number_of_polys_in_object;i++)
				{
					(*current_poly).draw_code=3;	//transparent & translucent
					current_poly++;
				}
			}	//end of dynslot
		}
        else
        {	//dynamic
				static_slot=get_static_object_slot('PNT&');	// the center object
				if (static_slot==-1) report_error_c ("map_control:load_scenary couldn't find PNT&","",-5);
				dyn_slot=find_vacant_dynamic_object_slot();
				if (dyn_slot!=-1)
				{
				int squish;
					my_tex_ptr=get_memory_for_dyn_tex();
					if (my_tex_ptr==0)
					{
					  add_to_text_display("Zex Exec> WARNING: get_memory_for_dyn_tex failed in load_scenary",1);
					  return;
					}

					if (seed%6<3) squish=1;
					else  squish=0;

					clear_pb(&the_params);
					the_params.world_x=x;

					the_params.world_y=y;
					//the_params.rot_x=RangedRdm(30,88);

					if (squish==1)
					the_params.world_z=RangedRdm(0,2000)-1000;	//RangedRdm(2000,6000);
					else
					the_params.world_z=(ABS(seed*8))+50000;	//RangedRdm(2000,6000);

					//          if (the_params.world_z<121000) the_params.world_z=121000;
					the_params.Omovedx=0;
					the_params.Omovedy=0;
					the_params.Omovedz=0;

					//	if (squish==0) the_params.delta_rot_z=(seed&7)-3;	//+-3

					//the_params.rot_x=(seed%90)-45;	//+-45
					the_params.object_category=SCENARY;
					the_params.controller_ref=planet_rot_y;

					the_params.no_trivial_rejection=0;	//controlled when not in view
					the_params.who_spawned_this_object=-1;
					load_dyn_object(static_slot,dyn_slot,&the_params,galactic_id);	//the object, the position (-1=next free)
					//now set up the motherships memory
					current_object_ptr=&ocb_ptr->object_list[dyn_slot];
					//set up texture
					//get mem ofr otexture
					current_object_ptr->dynamic_tex_ptr=my_tex_ptr;
					//fill the texture with robs gfx
					galaxy_gen(seed,(unsigned char *) my_tex_ptr->tex_data,squish);

					#if OGL==1
					if (OGL_active==1)
					{
						err=aglGetError();
						if (err!=0) report_error_c ("load_scenary(1): AGL error.","",err);
						make_ogl_dynamic_texture(my_tex_ptr);
					}
					#endif
					//galaxy_gen(seed, my_tex_ptr->tex_data);
					//poke the texture into the object - we assume all polys are covered (they are)
					number_of_polys_in_object=current_object_ptr->number_of_polys;
					current_poly=current_object_ptr->polys_ptr;
					for (i=0;i<number_of_polys_in_object;i++)
					{
						(*current_poly).Ptexture=my_tex_ptr->tex_data;

						#if OGL==1
						(*current_poly).Pid=my_tex_ptr->name;
						#endif

						(*current_poly).draw_code=2;	//transparent
						current_poly++;
					}
				}	//end of dynslot ok
        }	//end of dynamic
}



//given a code and x and y load up the relevent objects for the code around the x and y
//for example an asteroid field
void load_special(float x, float y, int code, short galactic_id)
{
int number_of_items;
int it,static_slot,i;
load_dyn_object_pb the_params;
anim_pb anim;


if (code==ASTEROID_FIELD1 || code == ASTEROID_FIELD2)
{
number_of_items=code>>16;
static_slot=get_static_object_slot('AST1');	//go fetch fido (the object)
if (static_slot==-1) report_error_c("load_special: get_static_object_slot failed on AST1.","",-3);

for (i=0;i<number_of_items;i++)
{
 //loads in asteroids around x and y
 it=find_vacant_dynamic_object_slot();
 if (it!=-1)
  {
 //load in object
            clear_pb(&the_params);

 	 the_params.world_x=x+RangedRdm(0,40000)-20000;
 	 the_params.world_y=y+(RangedRdm(0,40000)-20000);
 	 the_params.world_z=MONSTER_Z;
 	 the_params.Omovedx=0;	 //RangedRdm(0,120)-140;
 	 the_params.delta_rot_y=RangedRdm(1,10)-5;
 	 the_params.delta_rot_z=RangedRdm(1,10)-5;
 	 the_params.controller_ref=persistent_asteroid;
 	 the_params.shield_value=31170;
 	 the_params.mass=11000;
 	 the_params.no_trivial_rejection=1;
 	  the_params.object_category=scenary_object;
      the_params.who_spawned_this_object=-1;
 	 load_dyn_object(static_slot,it,&the_params,galactic_id);	//the object, the position (-1=next free)
  }
}	//end of i
}
else
if (code==COWS1 || code == COWS2)
{
number_of_items=code>>16;
//slightly different here - we have to load an animated object
static_slot=get_static_object_slot('MN41');	//go fetch fido (the object)
if (static_slot==-1) report_error_c("load_Special: get_static_object_slot failed on MN41.","",-3);

for (i=0;i<number_of_items;i++)
{
 //loads in asteroids around x and y
 it=find_vacant_dynamic_object_slot();
 if (it!=-1)
  {
 //load in object
            clear_pb(&the_params);

 	 the_params.world_x=x+RangedRdm(0,40000)-20000;
 	 the_params.world_y=y+(RangedRdm(0,40000)-20000);
    anim.object_id[0]='MN41';
    anim.object_id[1]='MN42';
    anim.object_id[2]='MN43';
    anim.object_id[3]='MN44';
    anim.object_id[4]='MN43';
    anim.object_id[5]='MN42';

     the_params.has_roll=0;
	the_params.controller_ref=space_cow;
	the_params.shield_value=13070;
	the_params.mass=50;
	the_params.no_trivial_rejection=1;
	 the_params.object_category=2;

     the_params.who_spawned_this_object=-1;
    load_animated_object (&anim,4,3,1,1,&the_params,galactic_id);
  }
}	//end of i

}
//0.94
else
if (code==CORRAL_FACING_RIGHT)
  {
  int err,i,n;
  //build a right facing corral with excalibur guards
  //load cor_right.zob
  err=load_complex_object("cor_right.zob",static_cast<int>(x),static_cast<int>(y));
  if (err!=0)    add_to_text_display("Zex Exec: Warning couldn't load cor_right.zob",0);
// load n persistent cargo containers - CAR1 to 4 in static
  n=RangedRdm(3,12);
  for (i=0;i<n;i++)
      {
       int static_slot,colour,it;
       OSType id;

       colour=RangedRdm(1,4);

       if (colour==1) id='CAR1';
       else
       if (colour==2) id='CAR2';
       else
       if (colour==3) id='CAR3';
       else
       id='CAR1';

       static_slot=get_static_object_slot(id);
       if (static_slot==-1)    add_to_text_display("Zex Exec: Warning: CORRAL_FACING_RIGHT: Couldn't find cargo container. ",0);
       else
          {
          //load the object
           it=find_vacant_dynamic_object_slot();
           if (it!=-1)
           {
 //load in object
            clear_pb(&the_params);

 	     the_params.world_x=x-RangedRdm(0,20000)-10000;
 	     the_params.world_y=y-(RangedRdm(0,20000)-10000);
 	     the_params.world_z=MONSTER_Z;
 	     the_params.delta_rot_y=RangedRdm(1,10)-5;
 	     the_params.delta_rot_z=RangedRdm(1,10)-5;
 	     the_params.controller_ref=PERSISTENT_CARGO_CONTAINER;
	     the_params.shield_value=470;
	     the_params.mass=100;
	     the_params.no_trivial_rejection=1;
	     the_params.object_category=ALIEN;

         the_params.who_spawned_this_object=-1;
 	 load_dyn_object(static_slot,it,&the_params,galactic_id);	//the object, the position (-1=next free)
           }
          }	//end of good static slot


      }
  }

}


//search the OCB's fora given galactic_id, returns 0 if not found, else the slot number
int find_galactic_id_in_ocbs(short the_id)
{
extern	DynObjectsFixedSize *ocb_ptr;	//the dynamic objects
extern int	dynamic_object_index;

int i,active_object_count;

i=active_object_count=0;
while (active_object_count<dynamic_object_index  && i<MAX_NUMBER_OF_DYN_OBJECTS)
{
  if ((*ocb_ptr).object_list[i].in_use==1)
  {
   if ((*ocb_ptr).object_list[i].galactic_id==the_id)
   {
     return i;
     break;
   }
   active_object_count++;
 }
 i++;
}
return 0;




}

void load_planet(float x, float y, int seed, short galactic_id,int is_inhabited)
{
int static_slot,dyn_slot;

register ZObject * current_object_ptr;
load_dyn_object_pb the_params;
dynamic_texture * my_tex_ptr;
_ZPoly *current_poly;
int number_of_polys_in_object,i,err;
            Ptr scratch_mem_ptr;

//calc section rect
          my_tex_ptr=get_memory_for_dyn_tex();
          if (my_tex_ptr==0)

          {
          add_to_text_display("Zex Exec> WARNING: get_memory_for_dyn_tex failed in load_planet",1);
          return;
          }
//we use PNTZ, a dummy rectangle as our planet.
//we poke the address of our gen on the fly texture into all the polys
//planets live between 30 and 120 thousand Z
          scratch_mem_ptr=static_cast<char *>(malloc((256*256)+(128*128)+300));  // (1024*1024+300);
           if (scratch_mem_ptr==0)
          {
          add_to_text_display("Zex Exec> WARNING: load_planet couldn't get scratch memory. Graphic not loaded",1);
          return;
          }

        static_slot=get_static_object_slot('PNTZ');	// the center object
        if (static_slot==-1) report_error_c ("map_control:load_planet couldn't find PNTZ","",-5);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {
          clear_pb(&the_params);
	  the_params.world_x=x;

	  the_params.world_y=y;
	  the_params.world_z=ABS(seed*2);	//RangedRdm(2000,6000);
          if (the_params.world_z<30000) the_params.world_z=30000;
          if (the_params.world_z>120000) the_params.world_z=120000;
	  the_params.Omovedx=0;
	  the_params.Omovedy=0;
	  the_params.Omovedz=0;

	  the_params.delta_rot_y=0;
	  the_params.rot_z=seed%358;
	  the_params.object_category=SCENARY;
	  the_params.controller_ref=planet_rot_y;

	  the_params.no_trivial_rejection=1;	//controlled when not in view
          the_params.who_spawned_this_object=-1;
	  load_dyn_object(static_slot,dyn_slot,&the_params,galactic_id);	//the object, the position (-1=next free)
	  //now set up the motherships memory
	  current_object_ptr=&ocb_ptr->object_list[dyn_slot];
//set up texture
//get mem ofr otexture
          current_object_ptr->dynamic_tex_ptr=my_tex_ptr;
          if (current_object_ptr!=0)
            {

            //fill the texture with robs gfx

            if (scratch_mem_ptr!=0)
            {
             planet_gen(seed,is_inhabited,(unsigned char*) scratch_mem_ptr,(unsigned char*) my_tex_ptr->tex_data);
              #if OGL==1
             if (OGL_active==1)
             {
                 err=aglGetError();
                 if (err!=0) report_error_c ("load_planet(1): AGL error.","",err);
                 make_ogl_dynamic_texture(my_tex_ptr);
              }
              #endif
            }


            //galaxy_gen(seed, my_tex_ptr->tex_data);
            //poke the texture into the object - we assume all polys are covered (they are)
            number_of_polys_in_object=current_object_ptr->number_of_polys;
            current_poly=current_object_ptr->polys_ptr;
            for (i=0;i<number_of_polys_in_object;i++)
              {
              (*current_poly).Ptexture=my_tex_ptr->tex_data;

              #if OGL==1
              (*current_poly).Pid=my_tex_ptr->name;
              #endif


              (*current_poly).draw_code=2;	//transparent
              current_poly++;
              }
            }

        }
            free(scratch_mem_ptr);

}

void load_mothership(mothership_block * the_block)
{
int allegiance,static_slot,dyn_slot;
float x,y;
int shield_val;

load_dyn_object_pb the_params;
mothership_block * mship_mem_ptr;
register ZObject * current_object_ptr;

 allegiance=the_block->allegiance;
 shield_val=the_block->shield_value;
 x=the_block->real_coords.x;
 y=the_block->real_coords.y;
 if (allegiance==HUMAN)
    {
        static_slot=get_static_object_slot('MS02');	// the center object
        if (static_slot==-1) report_error_c ("map_control:load_mothership couldn't find ms02","",-4);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {
          clear_pb(&the_params);
	  the_params.world_x=x;
	  the_params.shield_value=shield_val;

	  the_params.world_y=y;
	  the_params.world_z=0;	//RangedRdm(2000,6000);
	  the_params.Omovedx=0;
	  the_params.Omovedy=0;
	  the_params.Omovedz=0;

	  the_params.delta_rot_y=0;
	  the_params.rot_y=0;
	  the_params.object_category=HUMAN;
	  the_params.controller_ref=GAME_MOTHERSHIP;
	  the_params.object_category=zex_object;

	  the_params.no_trivial_rejection=1;	//controlled when not in view
          the_params.who_spawned_this_object=-1;
	  load_dyn_object(static_slot,dyn_slot,&the_params,-1);	//the object, the position (-1=next free)
	  //now set up the motherships memory
	  current_object_ptr=&ocb_ptr->object_list[dyn_slot];
	  current_object_ptr->mship_ptr=the_block;	//very important!

        }
      //load in left arm
        static_slot=get_static_object_slot('MS01');	// the center object
        if (static_slot==-1) report_error_c ("map_control:load_mothership couldn't find ms01","",-4);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {
          clear_pb(&the_params);
	  the_params.world_x=x-2500;
	  the_params.shield_value=shield_val;

	  the_params.world_y=y;
	  the_params.world_z=0;	//RangedRdm(2000,6000);
	  the_params.Omovedx=0;
	  the_params.Omovedy=0;
	  the_params.Omovedz=0;
	  the_params.object_category=HUMAN;

	  the_params.delta_rot_x=1;
	  the_params.rot_y=0;

	  the_params.controller_ref=0;
	  the_params.object_category=zex_object;

	  the_params.no_trivial_rejection=1;	//controlled when not in view
          the_params.who_spawned_this_object=-1;
	  load_dyn_object(static_slot,dyn_slot,&the_params,-1);	//the object, the position (-1=next free)

        }
      //load in right arm
        static_slot=get_static_object_slot('MS01');	// the center object
        if (static_slot==-1) report_error_c ("map_control:load_mothership couldn't find ms01","",-4);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {
          clear_pb(&the_params);
	  the_params.world_x=x+2500;

	  the_params.world_y=y;
	  the_params.world_z=0;	//RangedRdm(2000,6000);
	  the_params.Omovedx=0;
	  the_params.Omovedy=0;
	  the_params.Omovedz=0;
	  the_params.shield_value=shield_val;
	  the_params.object_category=HUMAN;

	  the_params.delta_rot_x=-1;
	  the_params.rot_y=0;

	  the_params.controller_ref=0;
	  the_params.object_category=zex_object;

	  the_params.no_trivial_rejection=1;	//controlled when not in view
          the_params.who_spawned_this_object=-1;
	  load_dyn_object(static_slot,dyn_slot,&the_params,-1);	//the object, the position (-1=next free)

        }

    }
 else	//alien mothership
    {
        static_slot=get_static_object_slot('MS10');	// the center object
        if (static_slot==-1) report_error_c ("map_control:load_mothership couldn't find MS10","",-4);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {
          clear_pb(&the_params);
	  the_params.world_x=x;
	  the_params.shield_value=shield_val;

	  the_params.world_y=y;
	  the_params.world_z=0;	//RangedRdm(2000,6000);
	  the_params.Omovedx=0;
	  the_params.Omovedy=0;
	  the_params.Omovedz=0;

	  the_params.object_category=ALIEN;

	  the_params.delta_rot_y=-1;
	  the_params.rot_y=0;

	  the_params.controller_ref=GAME_MOTHERSHIP;
	  the_params.object_category=baddie_object;

	  the_params.no_trivial_rejection=1;	//controlled when not in view
          the_params.who_spawned_this_object=-1;
	  load_dyn_object(static_slot,dyn_slot,&the_params,-1);	//the object, the position (-1=next free)
	  //now set up the motherships memory
	  current_object_ptr=&ocb_ptr->object_list[dyn_slot];
	current_object_ptr->mship_ptr=the_block;	//very important!

        }
      //load in center section
        static_slot=get_static_object_slot('MS13');	// the center object
        if (static_slot==-1) report_error_c ("map_control:load_mothership couldn't find ms13","",-4);
        dyn_slot=find_vacant_dynamic_object_slot();
        if (dyn_slot!=-1)
        {
          clear_pb(&the_params);
	  the_params.world_x=x;
	  the_params.shield_value=shield_val;
	  the_params.object_category=SCENARY;

	  the_params.world_y=y;
	  the_params.world_z=0;	//RangedRdm(2000,6000);
	  the_params.Omovedx=0;
	  the_params.Omovedy=0;
	  the_params.Omovedz=0;

	  the_params.delta_rot_y=6;
	  the_params.delta_rot_x=0;
	  the_params.rot_y=0;

	  the_params.controller_ref=0;
	  the_params.object_category=ALIEN;

	  the_params.no_trivial_rejection=1;	//controlled when not in view
          the_params.who_spawned_this_object=-1;
	  load_dyn_object(static_slot,dyn_slot,&the_params,-1);	//the object, the position (-1=next free)

        }

    }

}




void set_map_update_rate(int rate)
{
map_update_count=rate;
}

//given an x and y return the section
int get_section(int quad, int x, int y)
{
int section_x,section_y,section;

 if (quad==1 || quad==3) x-=GALAXY_WIDTH/2;
 section_x=x/SECTION_SIZE;	//0-9 hopefully

 //normalise y for quad 0 and
 if (quad==0 || quad==1) y-=GALAXY_WIDTH/2;
 y=GALAXY_WIDTH/2-y;	//invert so min is at top (top is GALAXY_WIDTH)
 section_y=y/SECTION_SIZE;

     section=section_y*NUMBER_OF_SECTIONS_IN_A_QUAD_Y+section_x;
return section;
}

// <<===============================WARPHOLE=================================>> //

static int sd_counter=0;
void run_warphole()
{
	extern float dest_x,dest_y;	//from level file
	extern	DynObjectsFixedSize *ocb_ptr;
	extern int camera_rotation_z;
	extern float depth_of_field;
	extern int camera_rotation_z;	//obviously the camera rot
	extern int camera_rotation_x;
	extern int camera_rotation_y;
	extern int credits,hull_strength;
	extern int promotion_points;
	extern kit_desc ship_items[ITEMS_COUNT];

	int slot,it;
	int new_quad_x,new_quad_y;
	int hull_damage;

	int hole_dist;
	int ship_x,ship_y,obj_x,obj_y;
	int camy,camx,delta_x,delta_y;
	load_dyn_object_pb the_params;

	ship_x=static_cast<int>((*ocb_ptr).object_list[0].Oworldx);
	ship_y=static_cast<int>((*ocb_ptr).object_list[0].Oworldy);


	if (entered_hole>1)
	{
		int shield_damage;
		sd_counter++;	//shield damage in hole divider
		if (sd_counter>7)	//this has been tweaked so you can just get through a warphole with level 1 shields if they
		{	//are at 100% when you enter
		sd_counter=0;
		//ship_items[KSHIELDS].modifier=1; //use this to test
		//calc hull damage as damage-shield strength
			 shield_damage=6-ship_items[KSHIELDS].modifier;
			 hull_damage=ship_items[KSHIELDS].value1-shield_damage;;
			 if (hull_damage>0) hull_damage=0;
			 else
			 hull_damage=-hull_damage;

			 if (ship_items[KSHIELDS].value1>0)
			 {
				play_zsound(thud, sound_low_pri, c1_chan1, sound_vol_7);
				ship_items[KSHIELDS].value1=ship_items[KSHIELDS].value1-shield_damage;
			 }

			 if (hull_damage>0 && tutorial==0 )
			 {
					damage_me (hull_damage<<2);
				     if (hull_strength<=0)
					 {
						(*ocb_ptr).object_list[zex].in_use=1;
						ship_items[KFLATPACK].value1=0;	//flatpacks no good in warphole
						kill_zex();	//in ship control
					}
					damage_something();
			 }
		}
	}
	//gen shooting things and rotate camera
	if (end_scene==1)
	{




		if (current_quad==0)
		{	//set up for 1
			new_quad_x=NUMBER_OF_SECTIONS_IN_A_QUAD_X*SECTION_SIZE;
			new_quad_y=NUMBER_OF_SECTIONS_IN_A_QUAD_Y*SECTION_SIZE;
			old_quad=1; current_quad=1;
			play_zsound_always(message_from_command, sound_high_pri, c1_chan1, sound_vol_7);
			add_to_text_display("OBC> Message from Command: Congratulations on making it to Beta Quadrant!",2);
			add_to_text_display("OBC> Continues: Your account has been credited with 1000 credits.",2);
			credits+=1000;
			promotion_points+=100;
		}
		else
			if (current_quad==1)
			{	//set up for 2
				new_quad_x=0;
				new_quad_y=0;
				old_quad=2; current_quad=2;
				play_zsound_always(message_from_command, sound_high_pri, c1_chan1, sound_vol_7);
				add_to_text_display("OBC> Message from Command: Congratulations on making it to Gamma Quadrant!",2);
				add_to_text_display("OBC> Continues: Your account has been credited with 3000 credits.",2);
				credits+=3000;
				promotion_points+=300;
			}
		else
			if (current_quad==2)
			{	//set up for 3
				new_quad_x=NUMBER_OF_SECTIONS_IN_A_QUAD_X*SECTION_SIZE;
				new_quad_y=0;
				old_quad=3; current_quad=3;
				play_zsound_always(message_from_command, sound_high_pri, c1_chan1, sound_vol_7);
				add_to_text_display("OBC> Message from Command: Congratulations on making it to Delta Quadrant!",2);
				add_to_text_display("OBC> Continues: Your account has been credited with 5000 credits.",2);
				credits+=5000;
				promotion_points+=500;
				gone_around_flag=1;	//this is a saved global that says we have made it to the delta quadrant

			}
		else
			if (current_quad==3)
			{	//set up for 0
				new_quad_x=0;
				new_quad_y=NUMBER_OF_SECTIONS_IN_A_QUAD_Y*SECTION_SIZE;
				old_quad=0; current_quad=0;
				play_zsound_always(message_from_command, sound_high_pri, c1_chan1, sound_vol_7);
				add_to_text_display("OBC> Message from Command: Congratulations on navigating the whole Galaxy!",2);
				add_to_text_display("OBC> Continues: Your account has been credited with 30000 credits.",2);
				credits+=30000;
				promotion_points+=800;

			}
		else	// something has gone seriously wrong...
		{
			report_error_c("You appear to be in an unknown quadrant. Transfer to new quadrant failed.", "run_warphole()",current_quad);
			return;
		}

		quad_x=RangedRdm(5,65000)*300;
		quad_y=RangedRdm(5,65000)*300;
		quad_x+=new_quad_x;
		quad_y+=new_quad_y;

		//load up required objects which are warphole, zex and alien mothership
		clear_all_cont_sound_chans();
		play_zsound_always(explosion4, sound_high_pri, stereo1_chan, sound_vol_7);
		play_zsound_always(missile_explode, sound_high_pri, c1_chan1, sound_vol_7);
		play_zsound_always(screech, sound_high_pri, c1_chan2, sound_vol_7);
		//kill everything
		kill_all_things();
		//load Zex for game
		slot=get_static_object_slot('ZEXS');	//go fetch fido (the object)
		if (slot==-1) report_error_c ("Level4: Couldn't find ZEXS???","",-3);
		replace_dyn_object(slot,0);	//the object, the position (-1=next free)
									//load in warphole mouth
		slot=get_static_object_slot('WH1Z');	//go fetch fido (the object)
		if (slot==-1) report_error_c ("Level4: Couldn't find WH1Z","",-3);

		it=find_vacant_dynamic_object_slot();
		if (it!=-1)
		{
			//load in object
			clear_pb(&the_params);

			the_params.world_x=0;

			the_params.world_y=0;
			the_params.world_z=35000;	//RangedRdm(2000,6000);
			the_params.delta_rot_z=-1;

			the_params.object_category=scenary_object;

			the_params.no_trivial_rejection=1;	//controlled when not in view
			the_params.who_spawned_this_object=-1;
			load_dyn_object(slot,it,&the_params,-1);	//the object, the position (-1=next free)
		}


		//load in warphole core
		slot=get_static_object_slot('WH2&');	//go fetch fido (the object)
		if (slot==-1) report_error_c ("Level4: Couldn't find WH2&","",-3);

		it=find_vacant_dynamic_object_slot();
		if (it!=-1)
		{
			//load in object
			the_params.world_x=0;

			the_params.world_y=0;
			the_params.world_z=45000;	//RangedRdm(2000,6000);
			the_params.delta_rot_z=-1;
			the_params.object_category=scenary_object;

			the_params.no_trivial_rejection=1;	//controlled when not in view
			the_params.who_spawned_this_object=-1;
			load_dyn_object(slot,it,&the_params,-1);	//the object, the position (-1=next free)
		}

		//set camera
		camera_pos.x=0;	//quad_x;
		camera_pos.y=0;	//quad_y;
		camera_pos.z=0;

		//set zex vel in z
		(*ocb_ptr).object_list[0].Omovedz=-400;
		(*ocb_ptr).object_list[0].Oworldz=35000;
		(*ocb_ptr).object_list[0].no_trivial_rejection=1;

		//level_has_camera_control=0;	//this'll cause much trashing in the camera controller no doubt...

		//// in_ramparian_space=1;	//global for engine (dust etc.)
		end_scene=2;
		return;
	}




	if (end_scene==2)
	{
		int slot,it;
		//flip zex if gone past
		if ((*ocb_ptr).object_list[0].Oworldz<400)
		{
			kill_all_things();
			(*ocb_ptr).object_list[0].Omovedz=400;
			(*ocb_ptr).object_list[0].Oroty=90;
			(*ocb_ptr).object_list[0].is_visible=1;

			(*ocb_ptr).object_list[0].Orotz=0;
			(*ocb_ptr).object_list[0].Orotx=0;
			camera_rotation_z=camera_rotation_y=camera_rotation_x=0;
			////load in excalibur <G>
			////load in warphole core
			// slot=get_static_object_slot('EXC1');	//go fetch fido (the object)
			// if (slot==-1) report_error ("Level4, ENDSCENE=2: Couldn't find EXC1","",-3);
			//
			//it=find_vacant_dynamic_object_slot();
			//if (it!=-1)
			// {
			//        clear_pb(&the_params);
			////load in object
			//	  the_params.world_x=30000;
			//
			//	  the_params.world_y=30000;
			//	  the_params.world_z=0;	//RangedRdm(2000,6000);
			//	  the_params.object_category=ALIEN;
			//        the_params.controller_ref=excalabur;
			//	   the_params.no_trivial_rejection=1;	  //controlled when not in view
			//         the_params.who_spawned_this_object=-1;
			//	  load_dyn_object(slot,it,&the_params,-1);	  //the object, the position (-1=next free)
			// }

			end_scene=3;
		}


		return;
	}

	if (end_scene==3)
	{
		load_shooter_1(star_slot);
		if ((*ocb_ptr).object_list[0].Omovedz>0) (*ocb_ptr).object_list[0].Omovedz-=4;
		if ((*ocb_ptr).object_list[0].Oroty!=0) (*ocb_ptr).object_list[0].Oroty--;
		if ((*ocb_ptr).object_list[0].Omovedz<=0)
		{
			end_scene=4;
			(*ocb_ptr).object_list[0].Omovedz=0;
			(*ocb_ptr).object_list[0].Oworldx=quad_x;
			(*ocb_ptr).object_list[0].Oworldy=quad_y;
			(*ocb_ptr).object_list[0].Oworldz=-100;
			//      level_has_camera_control=0;	//this'll cause much trashing in the camera controller no doubt...
			//force an OCB complete reload
			disable_camera_control=0;
			camera_pos.x=quad_x;
			camera_pos.y=quad_y;
			camera_pos.z=0;

		}
		return;
	}

	if (end_scene==4)
	{
		add_to_text_display("OBC> Please wait. Rebooting...",0);
		play_zsound_always(mac_startup, sound_high_pri, c1_chan1, sound_vol_7);

		can_gen_dust=1;
		thrust=0;
		timer_5=0;
		end_scene=5;
		return;
	}

	if (end_scene==5)
	{
		int slot,it;
		timer_5++;
		if (timer_5>190)
		{
			add_to_text_display("OBC> Regained ship control!",0);
			force_ocb_update();
			level_has_control=0;

			//  add_to_text_display("OBC> Found alien mothership. looks like we made it!",0);
			//  add_to_text_display("OBC> Dock with mothership to end mission.",0);
			////load alien mothership
			// slot=get_static_object_slot('MS10');	//go fetch fido (the object)
			// if (slot==-1) report_error ("Level4: Couldn't find MS10","",-3);
			//
			//it=find_vacant_dynamic_object_slot();
			//if (it!=-1)
			// {
			////load in object
			//        clear_pb(&the_params);
			//	  the_params.world_x=300000;
			//
			//	  the_params.world_y=-200000;
			//	  the_params.world_z=0;	//RangedRdm(2000,6000);
			//	  the_params.delta_rot_y=2;
			//
			//	  the_params.controller_ref=earth_mothership;
			//
			//	  the_params.shield_value=11170;
			//	  the_params.mass=90;
			//	  the_params.object_category=baddie_object;
			//
			//	   the_params.no_trivial_rejection=1;	  //controlled when not in view
			//         the_params.who_spawned_this_object=-1;
			//	  load_dyn_object(slot,it,&the_params,-1);	  //the object, the position (-1=next free)
			// }
			//
			//
			// slot=get_static_object_slot('MS11');	//go fetch fido (the object)
			// if (slot==-1) report_error ("Level4: Couldn't find MS11","",-3);
			//
			//it=find_vacant_dynamic_object_slot();
			//if (it!=-1)
			// {
			////load in object
			//        clear_pb(&the_params);
			//	  the_params.world_x=300000;
			//
			//	  the_params.world_y=-200000;
			//	  the_params.world_z=0;	//RangedRdm(2000,6000);
			//	  the_params.delta_rot_y=-2;
			//
			//	  the_params.controller_ref=earth_mothership;
			//
			//	  the_params.shield_value=11170;
			//	  the_params.mass=90;
			//	  the_params.object_category=baddie_object;
			//
			//	   the_params.no_trivial_rejection=1;	  //controlled when not in view
			//         the_params.who_spawned_this_object=-1;
			//	  load_dyn_object(slot,it,&the_params,-1);	  //the object, the position (-1=next free)
			//
			//load_motherships_into_nav();
			//
			// }
			////level_complete=2;
			////  dest_x=300000;
			////  dest_y=-200000;
			//     insert_batcom_message("BATCOM RESET   ","                ");
			////   can_do_next_level=1;
			game_running=1;
			end_scene=6;
			in_wormhole=0;	//set main render loop back to control panel
			reinit_screen();	//get cp back up
			warphole_slot=-1;

		}
		return;
	}

	//that's it folks! All done. Lashings of ginger ale all round!

	if (end_scene==6)
	{
		return;
	}


	if (entered_hole==5)
	{
		frame_count++;

		timer_6++;

		timer_5++;

		if (timer_5>2)
		{
			timer_5=0;
			camera_rotation_z+=cam_rot_d;
		}

		if (frame_count<400)
			load_shooter_1(shooter1_slot);

		if (frame_count>300 && frame_count<600)
		{
			if (rumble_volume>0)
			{
				rumble_volume-=1;
				set_cont_sound_chan_volume(warphole_slot,rumble_volume);	//fade out the rumble
				if (rumble_volume==1) clear_all_cont_sound_chans();	//so we know what state we're in
			}

			cam_rot_d=-cam_rot_d;
			load_shooter_1(shooter2_slot);
		}

		//stars and lines
		if (frame_count>600 && frame_count<1200)
		{
			if (timer_6>4)	//flash screen
			{
				timer_6=0;
				if (RangedRdm(0,100)>50)
					set_erase_colour(255);
				else
					set_erase_colour(246);

				//  bg_colour--;
				//  if (bg_colour<244) bg_colour=255;
			}

			load_star(star_slot);
			//  load_star(star_slot);

			load_shooter_1(shooter1_slot);
			// if (RangedRdm(0,100)>95) load_shooter_1(shooter1_slot);
		}

		if (frame_count>1199 && frame_count<1212)
		{
			//       load_circle(galaxy_slot);
			set_erase_colour(255);
			load_sphere(ball1_slot,sphere_rot);
			sphere_rot+=33;
		}
		if (frame_count==1212) load_galaxy(gala_slot);

		if (frame_count==1220) check_balls();	//check if registered - will end the game if not

		if (frame_count>1280)
		{
			int slot;
			slot=find_dynamic_object_slot('BLL1');
			if (slot!=-1)
			{
				(*ocb_ptr).object_list[slot].Omovedz=-1200;
			}
			if (RangedRdm(0,100)>94) load_ring(ring_slot);

		}

		if (frame_count==1330) play_simple_continuous_sound(tractor_beam_loop, 40, 1);
		if (frame_count>1332)
		{
			inc_midi--;
			if (inc_midi<0)
			{
				inc_midi=17;
				set_cont_sound2_note(midi_note);
				midi_note++;
				if (midi_note>68)
				{
					end_scene=1;	//get out
					entered_hole=0;
				}
			}
		}
		//camera_pos.y=0; camera_pos.x=0;

		if (frame_count<1000) camera_pos.y+=cy_delta;
		else
		{
			if (camera_pos.y>0) camera_pos.y--;
			if (camera_pos.y<0) camera_pos.y++;

		}
		if (camera_pos.y>1000) cy_delta=-cy_delta;
		if (camera_pos.y<-1000) cy_delta=-cy_delta;

		//if (cy_delta<60) cy_delta++;

		if (camera_rotation_z>359) camera_rotation_z-=360;
		if (camera_rotation_z<0) camera_rotation_z+=360;

		return;
	}

	//zoom zex off int odistance and kill it
	if (entered_hole==4)
	{
        (*ocb_ptr).object_list[0].Oworldz+=220;
		if ((*ocb_ptr).object_list[0].Oworldz>6000)
		{
			entered_hole=5;
			kill(0);

		}

		return;
	}


	if (entered_hole==3)
	{
		int static_slot;

		//we are now through the mouth and tunnel and in blackness
		//lets get everything back to normal
		depth_of_field=256;	//back to normal on the camera lens
		(*ocb_ptr).object_list[0].Oworldz=-100;
		(*ocb_ptr).object_list[0].Oworldx=0;
		(*ocb_ptr).object_list[0].Oworldy=0;

		camera_pos.z=-1500;	//move cam back
		camera_pos.x=0;
		camera_pos.y=0;

        (*ocb_ptr).object_list[0].Oworldz+=220;

		//    static_slot=get_static_object_slot('ZEX1');	//go fetch fido (the object)
		//    if (static_slot==-1) report_error ("Level4: Couldn't find ZEX1.","",-3);
		//
		//    replace_dyn_object(static_slot,0);	//the object, the position (-1=next free)

		entered_hole=4;
		return;
	}

	if (entered_hole==1)
	{
		//stop_music_now();
		//clear_all_cont_sound_chans();
		play_zsound_always(space_synth, sound_high_pri, c1_chan1, sound_vol_7);
		entered_hole=2;
	}

	if (entered_hole==2)
	{
		int mouth_core_slot;

		//(*ocb_ptr).object_list[0].Orotdz=-15;
		(*ocb_ptr).object_list[warphole_trunk].Orotdz=7;

		//make the mouth invisible so we don't get funny old artifacts
		//  mouth_core_slot=find_dynamic_object_slot('WH1&');
		//  if (mouth_core_slot!=-1)
		//  {
		//  (*ocb_ptr).object_list[mouth_core_slot].eng_invisible=1;
		//  }
		(*ocb_ptr).object_list[0].Oworldz+=100;

		camy=static_cast<int>(-camera_pos.y);
		camx=static_cast<int>(-camera_pos.x);

		if ( camy < ship_y) camera_pos.y-=vib;
		else camera_pos.y+=vib;


		if ( camx < ship_x) camera_pos.x-=vib;
		else camera_pos.x+=vib;

		if (vib>0) vib-=3;

        (*ocb_ptr).object_list[0].Oworldz+=220;
        if (camera_pos.z<-100)
        {
			camera_pos.z+=20;
        }
        else
        {
			if (depth_of_field<3000) depth_of_field+=100;
			else	//we are now completely black
			{
				entered_hole=3;
				//kill the warp hole entrance
				//we have to move the sound from a 3d one to a cont one

				kill_all_things();
				camera_pos.y-=30;
				//          add_to_kill_list(warphole_core_slot);
				//          add_to_kill_list(warphole_trunk);

			}
        }
	}

	else	//haven't entered yet
	{
		//first, if zex is close enough to the target we start generating stars affected by the wh
		hole_dist = get_distance_to_from(warphole_core_slot,0);

		if (hole_dist<10000 && over_hole<2)
			over_hole=2;
		else
			if (hole_dist<30000 && over_hole<1)
				over_hole=1;
		if (hole_dist>30000)  over_hole=0;

		if (over_hole==1)
		{
			//OBC warnings
			if (ZexTickCount()>obc_warning_timer)
			{
				//audible warning
				play_zsound(danger, sound_high_pri, speech_chan, sound_vol_7);	   //do computer danger voice
				obc_warning_timer=ZexTickCount()+60*(hole_dist/10000+3);
			}

			//   gen_dust_timer++;
			//   if (gen_dust_timer>2)
			//       {
			//       //gen a piece of dust that flies into hole
			//       gen_dust_timer=0;
			//       gen_dust_for_warphole();
			//       }
		}

		if (over_hole==2)	//err we go..
		{
			int static_slot,toll;
			{
				toll=get_current_warphole_toll();
				if (toll<credits)	//we can afford it
				{
					if (gzex_prefs.registered==0)	//don't even consider warphole if we're not registered
					{
						if (done_warphole_message==0)
						{
							done_warphole_message=1;
							display_unreg(UNREG_WARPHOLE);
						}
					}
					else
					{
						fade_and_play_zex_resource(133);
						credits-=toll;
						can_gen_dust=0;
						add_to_text_display("OBC> Warphole toll has been paid",0);
						add_to_text_display("OBC> LOST SHIP CONTROL!",0);
						//    in_wormhole=1;	  //set main render loop to not show control panel
						//    reinit_screen();
						//    fade_out_music();
						over_hole=3;
						level_has_control=1;
						//    level_has_camera_control=1;
						disable_camera_control=1;
						//swap zex for zexc
						static_slot=get_static_object_slot('ZEX1');	//go fetch fido (the object)
						if (static_slot==-1) report_error_c ("Warphole: Couldn't find ZEX1.","",-3);

						replace_dyn_object(static_slot,0);	//the object, the position (-1=next free)
					}
				}
				else
				{
					if (ZexTickCount()>obc_warning_timer)
					{
						obc_warning_timer=ZexTickCount()+180*(hole_dist/10000+3);
						add_to_text_display("OBC> Entry to warphole denied.",0);
						add_to_text_display("OBC> We can't afford it.",0);

					}
				}
			}	//end of registered
		}

		if (over_hole==3)	//move ship over hole whilst zooming camera in
		{
			int centered;
			LSRect hole_box;


			gen_dust_timer++;
			if (gen_dust_timer>2)
			{
				//gen a piece of dust that flies into hole
				gen_dust_timer=0;
				gen_dust_for_warphole();
			}
			if ((*ocb_ptr).object_list[0].Omovedx>0) (*ocb_ptr).object_list[0].Omovedx--;
			if ((*ocb_ptr).object_list[0].Omovedx<0) (*ocb_ptr).object_list[0].Omovedx++;
			if ((*ocb_ptr).object_list[0].Omovedy>0) (*ocb_ptr).object_list[0].Omovedy--;
			if ((*ocb_ptr).object_list[0].Omovedy<0) (*ocb_ptr).object_list[0].Omovedy++;

			obj_x=static_cast<int>((*ocb_ptr).object_list[warphole_core_slot].Oworldx);
			obj_y=static_cast<int>((*ocb_ptr).object_list[warphole_core_slot].Oworldy);

			//see if we're centered
			hole_box.top=obj_y+200;
			hole_box.left=obj_x-200;
			hole_box.bottom=obj_y-200;
			hole_box.right=obj_x+200;

			if (point_in_rect(ship_x,ship_y,hole_box)==1)
			{
				centered=1;
			}
			else centered=0;


			delta_x=delta_y=0;

			//  if (ship_x<obj_x)
			delta_x=(obj_x-ship_x);
			//  if (ship_x>obj_x) delta_x=(ship_x-obj_x);

			//  if (ship_y<obj_y)
			delta_y=(obj_y-ship_y);
			//  if (ship_y>obj_y) delta_y=-(ship_y-obj_y);

			delta_x/=16;
			delta_y/=16;

			(*ocb_ptr).object_list[0].Omovedx=delta_x;
			(*ocb_ptr).object_list[0].Omovedy=delta_y;
			camy=static_cast<int>(-camera_pos.y);
			camx=static_cast<int>(-camera_pos.x);

			if (camera_pos.z<-800) camera_pos.z+=60;

			if ( camy < ship_y) camera_pos.y-=40;
			else camera_pos.y+=40;


			if ( camx < ship_x) camera_pos.x-=40;
			else camera_pos.x+=40;


			//if centered then rotate Zex to 90 degrees
			if (centered==1)
			{
				int ship_ang=(*ocb_ptr).object_list[0].Orotz;

				if (ship_ang!=90)
				{
					ship_ang++;
					if (ship_ang<0) ship_ang+=360;
					if (ship_ang>359) ship_ang-=360;
					(*ocb_ptr).object_list[0].Orotz=ship_ang;
				}

				int ship_x_ang=(*ocb_ptr).object_list[0].Oroty;
				if (ship_ang==90)
				{
					if (ship_x_ang!=270)
					{
						ship_x_ang--;
						if (ship_x_ang<0) ship_x_ang+=360;
						if (ship_x_ang>359) ship_x_ang-=360;
						(*ocb_ptr).object_list[0].Oroty=ship_x_ang;
					}
				}

				//we are now facing down the hole
				if (ship_x_ang==270 && ship_ang==90)
				{
					(*ocb_ptr).object_list[0].Oworldz+=70;
					if (camera_pos.z<-80)
					{
						camera_pos.z+=40;
					}
					else
					{
						int static_slot;

						add_to_text_display("OBC> SUFFERING A MAJOR ANXIETY ATTACK!",0);

						over_hole=-1;	//ok, we're in
						entered_hole=1;
						//swap zex1 for zex2 - brighter engines
						static_slot=get_static_object_slot('ZEX2');	//go fetch fido (the object)
						if (static_slot==-1) report_error_c ("Level4: Couldn't find ZEXC.","",-3);

						replace_dyn_object(static_slot,0);	//the object, the position (-1=next free)

						//	kill_all_things();	//kill all but zex and wormhole
					}
				}


			} // if centered == 1

		} // if(over hole == 3)

	} // we haven't entered

}

void kill_all_things()
{
int i;

for (i=1;i<MAX_NUMBER_OF_DYN_OBJECTS-1;i++)
   {
    if ((*ocb_ptr).object_list[i].in_use==1) kill(i);
   }
}

void load_shooter_1(int static_slot)
{
extern	DynObjectsFixedSize *ocb_ptr;
extern _3D ship_rotated_normal; //from engine
extern int thrust,number_of_stars,global_show_status;
extern _3D camera_pos;
int camera_z;

load_dyn_object_pb the_params;

int it;
float dust_random_x,dust_random_y;
//float ship_dir_x,ship_dir_y;
//oif (global_show_status!=0) return;

it=find_vacant_dynamic_object_slot();
if (it!=-1)
 {
//load in object
        clear_pb(&the_params);
        if (RangedRdm(0,100)>50)
	the_params.world_x=-300;
	else
	the_params.world_x=300;

	the_params.world_y=RangedRdm(1,4000)-2000;
	the_params.world_z=5000;	//RangedRdm(2000,6000);
	the_params.Omovedz=-RangedRdm(95,290);

	the_params.rot_y=90;

	the_params.controller_ref=STAR_FOR_WARPHOLE;

	the_params.object_category=scenary_object;

	 the_params.no_trivial_rejection=1;	//controlled when not in view
     the_params.who_spawned_this_object=-1;
	load_dyn_object(static_slot,it,&the_params,-1);	//the object, the position (-1=next free)


 }
}

void load_sphere(int static_slot, int rot_z)
{
extern	DynObjectsFixedSize *ocb_ptr;
extern _3D ship_rotated_normal; //from engine
extern int thrust,number_of_stars,global_show_status;
extern _3D camera_pos;
int camera_z;

load_dyn_object_pb the_params;

int it;
float dust_random_x,dust_random_y;
//float ship_dir_x,ship_dir_y;
//oif (global_show_status==1) return;

it=find_vacant_dynamic_object_slot();
if (it!=-1)
 {
//load in object
        clear_pb(&the_params);
	the_params.world_x=300;

	the_params.world_y=0;
	the_params.world_z=38000;	//RangedRdm(2000,6000);
	the_params.delta_rot_z=3;
	the_params.rot_z=rot_z;

	the_params.controller_ref=STAR_FOR_WARPHOLE;
	the_params.object_category=scenary_object;

	 the_params.no_trivial_rejection=1;	//controlled when not in view
     the_params.who_spawned_this_object=-1;
	load_dyn_object(static_slot,it,&the_params,-1);	//the object, the position (-1=next free)


 }
}

void load_galaxy(int static_slot)
{
extern	DynObjectsFixedSize *ocb_ptr;
extern _3D ship_rotated_normal; //from engine
extern int thrust,number_of_stars,global_show_status;
extern _3D camera_pos;
int camera_z;

load_dyn_object_pb the_params;

int it;
float dust_random_x,dust_random_y;
//float ship_dir_x,ship_dir_y;
//oif (global_show_status==1) return;

it=find_vacant_dynamic_object_slot();
if (it!=-1)
 {
//load in object
        clear_pb(&the_params);
	the_params.world_x=0;

	the_params.world_y=0;
	the_params.world_z=228000;	//RangedRdm(2000,6000);
	the_params.delta_rot_z=1;

	the_params.controller_ref=STAR_FOR_WARPHOLE;
	the_params.object_category=scenary_object;

	 the_params.no_trivial_rejection=1;	//controlled when not in view
     the_params.who_spawned_this_object=-1;
	load_dyn_object(static_slot,it,&the_params,-1);	//the object, the position (-1=next free)


 }
}

void load_ring(int static_slot)
{
extern	DynObjectsFixedSize *ocb_ptr;
extern _3D ship_rotated_normal; //from engine
extern int thrust,number_of_stars,global_show_status;
extern _3D camera_pos;
int camera_z;

load_dyn_object_pb the_params;

int it;
float dust_random_x,dust_random_y;
//float ship_dir_x,ship_dir_y;
//oif (global_show_status==1) return;

it=find_vacant_dynamic_object_slot();
if (it!=-1)
 {
//load in object
        clear_pb(&the_params);
	the_params.world_x=0;

	the_params.world_y=0;
	the_params.world_z=88000;	//RangedRdm(2000,6000);
	the_params.Omovedz=-500;

	the_params.delta_rot_z=1;

	the_params.controller_ref=STAR_FOR_WARPHOLE;
	the_params.object_category=scenary_object;

	 the_params.no_trivial_rejection=1;	//controlled when not in view
     the_params.who_spawned_this_object=-1;
	load_dyn_object(static_slot,it,&the_params,-1);	//the object, the position (-1=next free)


 }
}


void load_star(int static_slot)
{
extern	DynObjectsFixedSize *ocb_ptr;
extern _3D ship_rotated_normal; //from engine
extern int thrust,number_of_stars,global_show_status;
extern _3D camera_pos;
int camera_z;

load_dyn_object_pb the_params;

int it;
float dust_random_x,dust_random_y;
//float ship_dir_x,ship_dir_y;
//oif (global_show_status==1) return;
star_rot+=5;
if (star_rot>359) star_rot=0;

it=find_vacant_dynamic_object_slot();
if (it!=-1)
 {
//load in object
        clear_pb(&the_params);
	the_params.world_x=COS(star_rot)*500;

	the_params.world_y=SIN(star_rot)*500;
	the_params.world_z=7000;	//RangedRdm(2000,6000);
	the_params.Omovedx=0;
	the_params.Omovedy=0;
	the_params.Omovedz=-RangedRdm(25,110);

	the_params.delta_rot_y=5;
	the_params.rot_y=90;

	the_params.controller_ref=STAR_FOR_WARPHOLE;
	the_params.object_category=scenary_object;

	 the_params.no_trivial_rejection=1;	//controlled when not in view
     the_params.who_spawned_this_object=-1;
	load_dyn_object(static_slot,it,&the_params,-1);	//the object, the position (-1=next free)


 }


}



//loads up the initial background objects

//removed by Rob 17-5-2005 - can't be static and extern!
//static int shooter1_slot,shooter2_slot,star_slot,mon2_slot,ball1_slot,gala_slot,ring_slot;

void init_map_control()
{
extern int global_show_target_vector;

old_quad=-1;
old_section=-1;
ocbs_holding_which_section=-1;	//so we can tell whether we need unload
ocbs_holding_which_quad=-1;
current_quad=0;
global_show_target_vector=0;	//no target yet
next_frame_run=0;	//so ocb's will be loaded immediately on start up
gone_around_flag=0;
surrendered_flag=0;
script_timer1=TIME_TILL_MESSAGE1_FROM_DELTA;
script_timer2=0;
script_state1=DO_DELTA_MESSAGE1;
script_state2=0;

   shooter1_slot=get_static_object_slot('SHT1');	//go fetch fido (the object)
   shooter2_slot=get_static_object_slot('SHT2');	//go fetch fido (the object)
   star_slot=get_static_object_slot('STAR');	//go fetch fido (the object)
   mon2_slot=get_static_object_slot('MON2');	//go fetch fido (the object)
   ball1_slot=get_static_object_slot('BLL1');	//go fetch fido (the object)
   gala_slot=get_static_object_slot('GALA');	//go fetch fido (the object)
   ring_slot=get_static_object_slot('RING');	//go fetch fido (the object)

   if (shooter1_slot==-1) report_error_c ("init_map_control: Couldn't find sht1 ","",-3);
    if (shooter2_slot==-1) report_error_c ("init_map_control: Couldn't find sht2 ","",-3);
    if (star_slot==-1) report_error_c ("init_map_control: Couldn't find STAR ","",-3);
    if (mon2_slot==-1) report_error_c ("init_map_control: Couldn't find MON2 ","",-3);
    if (ball1_slot==-1) report_error_c ("init_map_control: Couldn't find BLL1 ","",-3);
    if (gala_slot==-1) report_error_c ("init_map_control: Couldn't find GALA ","",-3);
    if (ring_slot==-1) report_error_c ("init_map_control: Couldn't find RING ","",-3);


}




void render_frame(int width, int height);
void display_unreg(int type)
{
int prefs_x,prefs_y;

int button_state;
Point mouse_xy;
Handle picture_H;
LSRAW* the_picture;
extern _3D light_normal;
extern int freeze;
extern int Mode320, zex_alive,user_abort,death_frame_counter;
_3D light_normal_save;
int return_val, real_return_val, old_button_state;

	if (type==UNREG_WARPHOLE)
	{
		#if EXPIRE
		picture_H=GetZexPicture ('ZRAW',162);	//not registered beta
		if (picture_H==0) report_error_c("Resource missing: ZRAW 162","",4);
		#else
		picture_H=GetZexPicture ('ZRAW',163);	//not registered release
		if (picture_H==0) report_error_c("Resource missing: ZRAW 163","",4);
		#endif
	}
	else if(type == UNREG_LOAD)
	{
		picture_H=GetZexPicture ('ZRAW',164);	//not registered, can't load
		if (picture_H==0) report_error_c("Resource missing: ZRAW 164","",4);
	}
	else if(type == UNREG_PERIODIC_NAGSCREEN)
	{
		picture_H=GetZexPicture ('ZRAW',165);	//not registered, periodic nag screen
		if (picture_H==0) report_error_c("Resource missing: ZRAW 165","",4);
	}
	else // type == UNREG_CAPTAIN_LOAD ... we assume
	{
		picture_H=GetZexPicture ('ZRAW',166);	//not registered, can't load captain
		if (picture_H==0) report_error_c("Resource missing: ZRAW 166","",4);
	}
	LS_HLock(picture_H);

    prefs_x=120; prefs_y=50;
//	the_drawing_buffer=Get_back_buffer();
    LS_ShowCursor();
    light_normal_save=light_normal;
	light_normal.z=1.3;	//dim the scene
	freeze=1;	//in 3d_top

    ZexGetMouse (&mouse_xy);


//    DSpContext_GlobalToLocal (mDisplayContext,&mouse_xy);
    button_state=game_fire_button();
	while(button_state==1)
	{
		button_state=game_fire_button();
	}
	old_button_state=button_state;
	return_val=real_return_val=0;
	bool first_pass = true;

	while(real_return_val==0)
	{
		return_val=0;
		if (Mode320==1)
		render_frame(320,390>>1);	//in this file - calls all the needed 3d stuff
		else
		render_frame(640,390);	//in this file - calls all the needed 3d stuff

		run_dlp();
		do_control_panel_control();

		ZexGetMouse (&mouse_xy);
		the_picture = (LSRAW*)*picture_H;

		print_pict_scaled(the_picture,prefs_x,prefs_y,0,1);	//Print with transparency

		button_state=game_fire_button();
		Show_it();

		if(first_pass)
		{
			user_delay(120);		// at least 2 second wait
			first_pass = false;
		}
		if (button_state==1 && old_button_state==0)	//if up and was down
		real_return_val = 1;
		else
		real_return_val=0;
		old_button_state=button_state;

	}
    LS_DisposeHandle(picture_H);
	LS_HideCursor();
    light_normal=light_normal_save;	//restore the lighting
	freeze=0;
}
