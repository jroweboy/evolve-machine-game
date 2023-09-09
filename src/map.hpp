
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

struct ObjectSpawn {
    u8 object_id;
    
    s16 x;
    s16 y;
};

// A room is made up of 1 or 2 sections representing a single screen worth of data called a section
struct Room {
    // Hold the map position for the lead and side room
    // If there is no side room (meaning we are a 1x1 room) then it will be an invalid id
    u8 lead_id;
    u8 side_id;

    // The object type that drops when this room is cleared
    // Update this if the prize has been grabbed
    PrizeType prize;
};

struct Section {
    // When changing rooms, we go to a new section first, and use this ID to look up the room
    u8 room_id;

    // Store which nametable to draw this section into.
    u8 nametable;

    // An exit refers to the section ID for the new section that we are moving to
    // Store the list of exits for this room in this order
    //    0   
    // 3 [ ] 1
    //    2   
    std::array<u8, 4> exit;
    // When moving from room to room, we can use the GetDirection to find what side
    // of the next section to spawn on.

    std::array<ObjectSpawn, 6> objects;

};

// The global RAM allocation for the room that the player is entering.
extern Room room;
extern Section lead;
extern Section side;
