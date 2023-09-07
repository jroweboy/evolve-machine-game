
#pragma once

#include <array>

#include "common.hpp"


// This doesn't seem necessary. Consider removing it?
enum class RoomType : u8 {
    Root, // Starting room
    Normal, // Average room. Maybe small chance to have a secret?
    Leaf, // Intentionally a terminal room. Maybe always has a secret?
    Boss, // TODO? Always has the key prize?
};

enum class PrizeType : u8 {
    None,
    Key,
    Weapon,
};

// Should each exit type is numbered for variations? Thats the current thought, but
// there's probably a better way to roll variations of exit types. Maybe use a bitfield
enum class ExitType : u8 {
    Blocked,
    Open,
    KeyRequired,
    LevelExit,
};

// This struct matches the `check_collision` call from nesdoug
struct Hitbox {
    u8 screen_x;
    u8 screen_y;
    u8 width;
    u8 height;
};

// TODO this is pretty much just a basic "object" and it overlaps with the player struct too
struct Hazard {

    u8 metasprite;
    
    s16 x;
    s16 y;
    
    Hitbox hitbox;
};

struct Room {
    // TODO: Do we need this?
    RoomType room_type;

    PrizeType prize;

    // Hold the metatile for this type of exit
    std::array<ExitType, 6> exit;

    // All data for this hazard slot.
    std::array<Hazard, 10> hazard_slot;

    std::array<u8, 16> decoration;

};
