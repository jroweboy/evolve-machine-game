
#pragma once 

// #ifdef __cplusplus
// extern "C" {
// #endif

#ifdef NES
#include <peekpoke.h>
#include <fixed_point.h>
#include "header/speed_table.hpp"

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
// extern "C" void donut_decompress(const void* data);


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

constexpr u8 PALETTE_KITTY = 0;
constexpr u8 PALETTE_WEAPON = 1;
constexpr u8 PALETTE_ENEMY1 = 2;
constexpr u8 PALETTE_ENEMY2 = 3;

enum class PaletteSet : u8 {
    None,
    WeaponCube, // Also kitty palette
    WeaponDiamond,
    WeaponPyramid,
    WeaponSphere,
    Hamster,
    Count,
};

enum class ObjectType : u8 {
    Player,
    WeaponCube,
    WeaponDiamond,
    WeaponPyramid,
    WeaponSphere,
    WeaponCubeAtk1,
    WeaponCubeAtk2,
    WeaponCubeAtk3,
    WeaponDiamondAtk1,
    WeaponDiamondAtk2,
    WeaponDiamondAtk3,
    WeaponPyramidAtk1,
    WeaponPyramidAtk2,
    WeaponPyramidAtk3,
    WeaponSphereAtk1,
    WeaponSphereAtk2,
    WeaponSphereAtk3,
    Armadillo,
    Pidgey,
    HamsterBall,
    Count,
    // Noinit types of objects
    None = 0xff,
};

enum class Metasprite : u8 {
    None,
    KittyUp,
    KittyRight,
    KittyDown,
    KittyLeft,

    WeaponCube,
    WeaponCubeAtk1,
    WeaponCubeAtk2,
    WeaponCubeAtk3,
    WeaponDiamond,
    WeaponDiamondAtk1,
    WeaponDiamondAtk2,
    WeaponDiamondAtk3,
    WeaponPyramid,
    WeaponPyramidAtk1,
    WeaponPyramidAtk2,
    WeaponPyramidAtk3,
    WeaponSphere,
    WeaponSphereAtk1,
    WeaponSphereAtk2,
    WeaponSphereAtk3,

    ArmadilloUp,
    ArmadilloRight,
    ArmadilloDown,
    ArmadilloLeft,

    PidgeyUp,
    PidgeyRight,
    PidgeyDown,
    PidgeyLeft,

    HamsterUp,
    HamsterRight,
    HamsterDown,
    HamsterLeft,

    HamsterBallUp,
    HamsterBallRight,
    HamsterBallDown,
    HamsterBallLeft,

    Count,
};

enum Direction : u8 {
    None = 0,
    Up = 1 << 0,
    Right = 1 << 1,
    Down = 1 << 2,
    Left = 1 << 3,
    Error = 1 << 7,
};

enum State : u8 {
    Normal = 0x00,
    EquippedWeapon = 0x01,
    GroundedWeapon = 0x02,
    HamsterChasing = 0x03,
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
// extern void (*irq_pointer)();

/// During init, this is set to true if we have epsm. We can use this for custom
/// IRQ raster effects or something later, i dunno yet.
extern bool has_epsm;

/// Type of weapon we have equipped (or None if not equipped)
extern ObjectType equipped_weapon;

/// Type of weapons we have collected (or none if the slot is empty)
extern ObjectType weapon_inventory[4];


#define prg_rom_1 __attribute__((section(".prg_rom_1")))
#define prg_rom_2 __attribute__((section(".prg_rom_2")))
#define prg_fixed __attribute__((section(".prg_rom_fixed")))
#define noinline __attribute__((noinline))
#define noinit __attribute__((section(".noinit.late")))
#define zpnoinit __attribute__((section(".zp.noinit")))

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

