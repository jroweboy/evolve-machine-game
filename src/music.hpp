
#pragma once 


#include "common.hpp"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((leaf)) void play_song(u8 song);
__attribute__((leaf)) void play_sfx(u8 song);

namespace Music {
constexpr u8 INTRO = 1;
constexpr u8 TITLE_AMBIENCE = 2;
constexpr u8 BOSS = 3;
}


#ifdef __cplusplus
}
#endif

