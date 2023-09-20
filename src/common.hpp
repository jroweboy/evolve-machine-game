
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

enum class MainMode : u8 {
    TitleScreen = 0,
    GenerateDungeon = 1,
    // MapLoader = 2,
    GamePlay = 2,
    Pause = 0x80,
};

enum class GameMode : u8 {
    InGame = 0,
    MapLoader = 1,
    Pause = 0x80,
};

/// Defines the current scroll position
struct Camera {
    s16 x;
    s16 y;
};

extern Camera view;

/// MainMode determines which main task the game will run. 
extern MainMode main_mode;

/// RNG seed for this run. Set when the game starts
extern u32 seed;

/// Global timer that always ticks up even during game pause!
/// Caution: don't use for animation
extern _BitInt(24) global_timer;


/// Pointer to the IRQ function
extern void (*irq_pointer)();

/// During init, this is set to true if we have epsm. We can use this for custom
/// IRQ raster effects or something later, i dunno yet.
extern bool has_epsm;

#ifdef __cplusplus
}
#endif

