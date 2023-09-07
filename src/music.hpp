
#pragma once 


#include "common.hpp"

#ifdef __cplusplus
extern "C" {
#endif

enum class Song : u8 { 
    Intro = 1,
    TitleAmbience = 2,
    Boss = 3,
    Silence = 0xfd,
    StopMusic = 0xfe,
    InitEngine = 0xff,
};

enum Sfx {
    EPSMDetected = 1 << 0,
};

__attribute__((leaf)) void play_song(Song song);
__attribute__((leaf)) void play_sfx(Sfx song);


#ifdef __cplusplus
}
#endif

