
#pragma once

#include <array>

#include "common.hpp"
//#include "object.hpp"


// This doesn't seem necessary. Consider removing it?
// enum class RoomType : u8 {
//     Root, // Starting room
//     Normal, // Average room. Maybe small chance to have a secret?
//     Leaf, // Intentionally a terminal room. Maybe always has a secret?
//     Boss, // TODO? Always has the key prize?
// };

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

struct ObjectSpawn {
    ObjectType id;
    State state;
    s16 x;
    s16 y;
};

enum class SectionBase : u8 {
    Bottom = 0,
    Left = 1,
    Right = 2,
    Single = 3,
    StartDown = 4,
    StartUp = 5,
    Top = 6,
    Count,
};

enum class ScrollType : u8 {
    Single = 0,
    Horizontal = 1,
    Vertical = 2,
};

// A room is made up of 1 or 2 sections representing a single screen worth of data called a section
struct Room {
    // Hold the map position for the lead and side room
    // If there is no side room (meaning we are a 1x1 room) then it will be an invalid id
    u8 lead_id;
    u8 side_id;
    ScrollType scroll;
    // X, Y position of the top left corner of the whole room.
    s16 x;
    s16 y;

    // The object type that drops when this room is cleared
    // Update this if the prize has been grabbed
    PrizeType prize;
};

struct Section {
    // When changing rooms, we go to a new section first, and use this ID to look up the room
    u8 room_id;

    // Store which nametable to draw this section into.
    u8 nametable;

    // The nametable stores which side to draw the screen in, but it doesn't say what screen to draw.
    // This is an enum of what type of screen to draw.
    SectionBase room_base;

    // An exit refers to the object ID for the type of exit that is drawn.
    // Bit 7 (0x80) is set when the exit is closed off.
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
