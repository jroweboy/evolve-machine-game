
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
    // EPSMDetected = 0,
    Intro,
    DangerousCitty,
    DangerousBattle,
    // TitleAmbience,
    // CombatAmbience,
    Boss,
    Silence = 0xfd,
    StopMusic = 0xfe,
    InitEngine = 0xff,
};

enum Sfx {
    EPSMDetected = 1 << 0,
};


extern Song next_song;
// __attribute__((leaf)) void play_song(Song song);
// __attribute__((leaf)) void play_sfx(Sfx song);


#ifdef __cplusplus
}
#endif

