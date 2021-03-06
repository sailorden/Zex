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

// ship_control.h
void apply_control_inputs();
//oldmac//short check_key(unsigned short k);
void ship_init();
void do_player_fire();
void reset_ship();
void reset_ship_with_carry_over();
//oldmac//short fire_button();
void do_player_laser();
void do_player_missile();
void kill_zex();


int get_pylon();
void select_next_pylon();
void reset_screen_from_status();
void clear_down_ship_on_eject();
void do_ejection();
void do_player_rear_laser();

void end_game_right_here_right_now(int EOLCode, const char * the_reason_string);

void do_shockwave(int modifier);
void do_warp_jump(int modifier);
void handle_ecm(unsigned int for_whom);
void run_missile_factory(void);

void damage_object_due_to_shockwave(int distance, int damage);
