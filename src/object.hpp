
#pragma once

#include <soa.h>
#include <fixed_point.h>

using namespace fixedpoint_literals;

#include "common.hpp"

constexpr u8 OBJECT_COUNT = 16;
constexpr u8 SOLID_OBJECT_COUNT = 32;

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
    None = 0,
    Solid = 1 << 0,
    Damage = 1 << 1,
    Pickup = 1 << 2,
    Enemy = 1 << 3,
    Bullet = 1 << 4,
    All = 0xff,
};

struct Object {
    f16_8 x;
    f16_8 y;
    
    Hitbox hitbox;
    
    /// CollisionType parameters for this object
    u8 collision;

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

    /// Currently used only to set the palette of the object
    u8 attribute;

    Direction direction;
    u8 speed;

    s8 hp;
    u8 atk;

    /// Counter for number of iframes this object has.
    u8 iframe;

    /// type of object
    ObjectType type;

} __attribute__((packed));

#define SOA_STRUCT Object
#define SOA_MEMBERS \
    MEMBER(metasprite) MEMBER(type) MEMBER(state) MEMBER(tile_offset) MEMBER(animation_frame) MEMBER(frame_counter) \
    MEMBER(direction) MEMBER(speed) MEMBER(attribute) MEMBER(hp) MEMBER(atk) MEMBER(x) MEMBER(y) MEMBER(hitbox) \
    MEMBER(collision) MEMBER(iframe)
#include <soa-struct.inc>

extern soa::Array<Object, OBJECT_COUNT> objects;

struct ObjectInitData {
    Metasprite metasprite;
    ObjectType type;
    Hitbox hitbox;
    State state;
    u8 collision;
    u8 attribute;
    s8 hp;
    u8 atk;
};
#define SOA_STRUCT ObjectInitData
#define SOA_MEMBERS \
    MEMBER(metasprite) MEMBER(hitbox) MEMBER(hp) MEMBER(atk) MEMBER(collision) MEMBER(attribute) \
    MEMBER(state)
#include <soa-struct.inc>


extern ObjectInitData object_init_data[(u8)ObjectType::Count];

struct SolidObject {
    CollisionType state;
    s16 x;
    s16 y;
    u8 width;
    u8 height;
} __attribute__((packed));
#define SOA_STRUCT SolidObject
#define SOA_MEMBERS MEMBER(state) MEMBER(x) MEMBER(y) MEMBER(width) MEMBER(height)
#include <soa-struct.inc>

// Array of current room solid objects
extern soa::Array<SolidObject, SOLID_OBJECT_COUNT> solid_objects;
extern u8 solid_object_offset;

// Data for the basic room layouts
// 4 corners
// constexpr u8 ROOM_WALL_COUNT = 2 * 4;
// extern SolidObject updown_walls[ROOM_WALL_COUNT];
// extern SolidObject leftright_walls[ROOM_WALL_COUNT];
// extern SolidObject single_walls[ROOM_WALL_COUNT];
// extern const soa::Array<SolidObject, ROOM_WALL_COUNT> updown_walls;
// extern const soa::Array<SolidObject, ROOM_WALL_COUNT> leftright_walls;
// extern const soa::Array<SolidObject, ROOM_WALL_COUNT> single_walls;
