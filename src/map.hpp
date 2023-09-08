
#pragma once

#include <array>

#include "common.hpp"


// This doesn't seem necessary. Consider removing it?
// enum class RoomType : u8 {
//     Root, // Starting room
//     Normal, // Average room. Maybe small chance to have a secret?
//     Leaf, // Intentionally a terminal room. Maybe always has a secret?
//     Boss, // TODO? Always has the key prize?
// };

// enum class 

enum class PrizeType : u8 {
    None,
    Key,
    Weapon,
};

// Should each exit type is numbered for variations? Thats the current thought, but
// there's probably a better way to roll variations of exit types. Maybe use a bitfield
enum ExitType {
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

struct SpriteSpawn {
    u8 object_id;
    
    s16 x;
    s16 y;
};

struct Room {
    // TODO: Do we need this?
    // RoomType room_type;
    
    // Hold the room_id for each exit for this and the buddy room.
    // I think this is only useful for map generation? Doesn't hurt either way
    u8 room_id;
    u8 position;
    u8 buddy;

    PrizeType prize;

    std::array<u8, 4> main_exit;
    std::array<u8, 4> buddy_exit;

    // All data for this hazard slot.
    std::array<SpriteSpawn, 8> hazard_slot;

    std::array<u8, 16> decoration;

};

// The global RAM allocation for the current room.
extern Room room;
