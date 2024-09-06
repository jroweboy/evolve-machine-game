
#pragma once 

// #ifdef __cplusplus
// extern "C" {
// #endif

#ifdef NES
#include <peekpoke.h>
#include <fixed_point.h>

using namespace fixedpoint_literals;
#else
#define fu8_8 u16
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
extern "C" void donut_decompress(const void* data);

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

enum class ObjectType : u8 {
    Player,
    WeaponSphere,
    WeaponPyramid,
    WeaponDiamond,
    WeaponCube,
    Count,
};

enum class Metasprite : u8 {
    KittyRight,
    KittyUp,
    KittyDown,
    KittyLeft,

    WeaponSphere,
    WeaponPyramid,
    WeaponDiamond,
    WeaponCube,

    Count,
};

enum class Direction : u8 {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3,
    Error = 0xff,
};

enum State : u8 {
    Normal = 0x00,
    EquippedWeapon = 0x01,
    GroundedWeapon = 0x02,
    Hidden = 0x80,
    Dead = 0xff,
};

extern u8 view_x;
extern u8 view_y;

/// MainMode determines which main task the game will run. 
extern MainMode main_mode;

/// RNG seed for this run. Set when the game starts
extern u32 seed;

#ifdef NES
/// Global timer that always ticks up even during game pause!
/// Caution: don't use for animation
extern _BitInt(24) global_timer;
#endif

/// Pointer to the IRQ function
extern void (*irq_pointer)();

/// During init, this is set to true if we have epsm. We can use this for custom
/// IRQ raster effects or something later, i dunno yet.
extern bool has_epsm;

/// Tracks the current offset to write the BG CHR tiles to for the offset.
/// When adding the CHR tiles, this is a convenient way to know what the last VRAM address is
/// so we can draw the next CHR tiles here and move it forward after.
extern u16 bg_chr_offset;
/// When generating the dungeon map, we want to make sure we don't spawn things that
/// over flow the chr count, so this is used in each room to make sure that its within
/// that parameter. This is also written to the object data itself, so that the metasprite rendering
/// code can offset the tile in the metasprite by the location it is in CHR-RAM
extern u8 bg_chr_count;

/// Same as `bg_chr_offset` but for the CHR in 0x1000
extern u16 sp_chr_offset;
extern u8 sp_chr_count;

#define prg_rom_1 __attribute__((section(".prg_rom_1")))
#define prg_rom_2 __attribute__((section(".prg_rom_2")))
#define noinit __attribute__((section(".noinit.late")))

constexpr u8 MUSIC_BANK = 0;
constexpr u8 GRAPHICS_BANK = 1;
constexpr u8 CODE_BANK = 2;
constexpr u8 FIXED_BANK = 3;

 #define MMAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MMIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _b : _a; })

// extern volatile char PPUMASK_VAR;

#define _DEBUGGER_0() { POKE(0x4018, 0); }
#define _DEBUGGER_1(a) { POKE(0x4018, (u8)a); }
#define _DEBUGGER_X(x,A,FUNC,...)  FUNC
#define DEBUGGER(...) _DEBUGGER_X(,##__VA_ARGS__,\
  _DEBUGGER_1(__VA_ARGS__),\
  _DEBUGGER_0(__VA_ARGS__))

extern "C" void __putchar(char c);

#define NTADR(x, y) (((y) << 5) | (x))

// #ifdef __cplusplus
// }
// #endif

