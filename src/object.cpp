
// TODO: move this to a spreadsheet or something i dunno.

#include "object.hpp"
#include "common.hpp"

prg_rom_1 ObjectInitData object_init_data[(u8)ObjectType::Count] = {
    {
        .metasprite = Metasprite::KittyLeft,
        .hitbox = { .x = 0, .y = 8, .width = 8, .height = 8},
        .state = State::Normal,
        .collision = 0,
        .attribute = 0,
        .hp = 3,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphere,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramid,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamond,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCube,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
};

// constexpr u8 WALL_SHORT = 16;
// constexpr u8 WALL_LONG = 100;

// prg_rom_2 SolidObject updown_walls[ROOM_WALL_COUNT] = {
//     // top left corner
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = WALL_LONG, .height = WALL_SHORT,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = WALL_SHORT, .height = WALL_LONG,
//     },
//     // top right corner
//     {
//         .state = CollisionType::Solid,
//         .x = 256 - WALL_LONG, .y = 0, .width = WALL_LONG, .height = WALL_SHORT,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 256 - WALL_SHORT, .y = 0, .width = WALL_SHORT, .height = WALL_LONG,
//     },
//     // bot left corner
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 480 - WALL_SHORT, .width = WALL_LONG, .height = WALL_SHORT,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 480 - WALL_LONG, .width = WALL_SHORT, .height = WALL_LONG,
//     },
//     // bot right corner
//     {
//         .state = CollisionType::Solid,
//         .x = 256 - WALL_LONG, .y = 480 - WALL_SHORT, .width = WALL_LONG, .height = WALL_SHORT,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 256 - WALL_SHORT, .y = 480 - WALL_LONG, .width = WALL_SHORT, .height = WALL_LONG,
//     },
// };

// prg_rom_2 SolidObject leftright_walls[ROOM_WALL_COUNT] = {
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = 100, .height = 16,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = 16, .height = 100,
//     }
// };

// prg_rom_2 SolidObject single_walls[ROOM_WALL_COUNT] = {
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = 100, .height = 16,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = 16, .height = 100,
//     }
// };
