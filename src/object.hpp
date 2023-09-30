
#pragma once

#include <soa.h>

#include "common.hpp"

constexpr u8 OBJECT_COUNT = 12;
constexpr u8 SOLID_OBJECT_COUNT = 20;

enum class ObjectType : u8 {
    Player,
    // Walker,
    Count,
};

enum class Metasprite : u8 {
    KittyRight,
    KittyUp,
    KittyDown,
    KittyLeft,
    // Walker,
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
    Hidden = 0x80,
    Dead = 0xff,
};

struct Hitbox {
    s8 x;
    s8 y;
    u8 width;
    u8 height;
};

#define SOA_STRUCT Hitbox
#define SOA_MEMBERS MEMBER(x) MEMBER(y) MEMBER(width) MEMBER(height)
#include <soa-struct.inc>

enum CollisionType {
    Solid = 1 << 0,
    Damage = 1 << 1,
    All = 0xff,
    // TODO: i dunno what else should go here
};
struct Object {
    s16 x;
    s16 y;
    
    Hitbox hitbox;

    /// Offset for the metasprite to render this.
    Metasprite metasprite;

    /// Current status for the object. Meaning is dependent on what the object is
    /// Negative values indicate that this should not be rendered (if its a sprite)
    State state;

    /// When loading the sprite, we draw them in whatever order we find them,
    /// so the metasprite data needs to be offset by the value of the actual
    /// tile as it appears in CHR-RAM
    u8 tile_offset;

    /// Current frame of animation for this metasprite
    u8 animation_frame;
    s8 frame_counter;

    Direction direction;
    u8 speed;

    s8 hp;
    u8 atk;
};

#define SOA_STRUCT Object
#define SOA_MEMBERS \
    MEMBER(metasprite) MEMBER(state) MEMBER(tile_offset) MEMBER(animation_frame) MEMBER(frame_counter) \
    MEMBER(direction) MEMBER(speed) MEMBER(hp) MEMBER(atk) MEMBER(x) MEMBER(y) MEMBER(hitbox)
#include <soa-struct.inc>

extern soa::Array<Object, OBJECT_COUNT> objects;

struct ObjectInitData {
    Metasprite metasprite;
    Hitbox hitbox;
    s8 hp;
    u8 atk;
};
#define SOA_STRUCT ObjectInitData
#define SOA_MEMBERS \
    MEMBER(metasprite) MEMBER(hitbox) MEMBER(hp) MEMBER(atk)
#include <soa-struct.inc>


extern ObjectInitData object_init_data[(u8)ObjectType::Count];


struct SolidObject {
    CollisionType state;
    s16 x;
    s16 y;
    u8 width;
    u8 height;
};
#define SOA_STRUCT SolidObject
#define SOA_MEMBERS MEMBER(state) MEMBER(x) MEMBER(y) MEMBER(width) MEMBER(height)
#include <soa-struct.inc>

// Array of current room solid objects
extern soa::Array<SolidObject, SOLID_OBJECT_COUNT> solid_objects;

// Data for the basic room layouts
// 4 corners
constexpr u8 ROOM_WALL_COUNT = 2 * 4;
extern SolidObject updown_walls[ROOM_WALL_COUNT];
extern SolidObject leftright_walls[ROOM_WALL_COUNT];
extern SolidObject single_walls[ROOM_WALL_COUNT];
