
#pragma once 


#include "common.hpp"

#ifdef __cplusplus
extern "C" {
#endif

// enum class Song : u8 { 
//     EPSMDetected = 0,
//     Intro = 1,
//     DangerousCitty = 2,
//     DangerousBattle = 3,
//     TitleAmbience = 4,
//     CombatAmbience = 5,
//     Boss = 6,
//     Silence = 0xfd,
//     StopMusic = 0xfe,
//     InitEngine = 0xff,
// };


enum class Song : u8 { 
    EPSMDetected = 0,
    Intro,
    DangerousCitty,
    DangerousBattle,
    DangerousBattleTransition,
    TitleAmbience,
    CombatAmbience,
    Boss,
    Silence = 0xfd,
    StopMusic = 0xfe,
    InitEngine = 0xff,
};

enum class Sfx : u8 {
	tile_press,
	menu_upright,
	menu_downleft,
	menu_cancel,
	squeak,
	trinket_collect,
	health_pickup,
	powerup_collect,
	enemy_cry_1,
	enemy_cry_2,
	enemy_cry_3,
	enemy_hurt,
	enemy_die_small,
	enemy_die_big,
	player_hurt,
	weapon_charged_1,
	weapon_charged_2,
	weapon_charged_3,
	weapon_fire_1,
	weapon_fire_2,
	menu_confirm,
	menu_select,
	menu_fanfarestart,
};


extern volatile Song next_song;
extern volatile Sfx sfx_queue1;
extern volatile Sfx sfx_queue2;

// __attribute__((leaf)) void play_song(Song song);
// __attribute__((leaf)) void play_sfx(Sfx song);


#ifdef __cplusplus
}
#endif

