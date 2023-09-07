
#pragma once 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;

// global decompress function defined in asm
// This raw decompress function is defined at 0xc000
extern void donut_decompress(const void* data);

enum class GameMode : u8 {
    TitleScreen = 0,
    MapLoader = 1,
    GamePlay = 2,
    Pause = 0x80,
};

extern GameMode game_mode;
extern GameMode prev_game_mode;

extern u16 rng_seed;

#ifdef __cplusplus
}
#endif

