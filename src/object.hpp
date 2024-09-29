
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


struct XYMagnitude {
    fs8_8 x;
    fs8_8 y;
};
#define SOA_STRUCT XYMagnitude
#define SOA_MEMBERS MEMBER(x) MEMBER(y)
#include <soa-struct.inc>

enum CollisionType {
    CollisionNone = 0,
    Solid = 1 << 0,
    Damage = 1 << 1,
    Pickup = 1 << 2,
    Enemy = 1 << 3,
    Bullet = 1 << 4,
    All = 0xff,
};

struct Object {
    fu16_8 x;
    fu16_8 y;
    
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

    u8 direction;
    u8 angle;
    Speed speed;

    s8 hp;
    u8 atk;

    /// Counter for number of iframes this object has.
    u8 iframe;

    /// type of object
    ObjectType type;

};

#define SOA_STRUCT Object
#define SOA_MEMBERS \
    MEMBER(metasprite) MEMBER(type) MEMBER(state) MEMBER(tile_offset) MEMBER(animation_frame) MEMBER(frame_counter) \
    MEMBER(direction) MEMBER(speed) MEMBER(attribute) MEMBER(hp) MEMBER(atk) MEMBER(x) MEMBER(y) MEMBER(hitbox) \
    MEMBER(collision) MEMBER(iframe) MEMBER(angle)
#include <soa-struct.inc>

extern soa::Array<Object, OBJECT_COUNT> objects;

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

struct SpeedTable {
    fu8_8 v;
    fu8_8 xy;
};
#define SOA_STRUCT SpeedTable
#define SOA_MEMBERS MEMBER(v) MEMBER(xy)
#include <soa-struct.inc>

extern const soa::Array<SpeedTable, (u8)Speed::Count> speed_table;


struct ObjectInitData {
    Metasprite metasprite;
    Hitbox hitbox;
    State state;
    Speed speed;
    u8 collision;
    u8 attribute;
    u8 hp;
    u8 atk;
    u8 frame_pacing;
};
#define SOA_STRUCT ObjectInitData
#define SOA_MEMBERS \
    MEMBER(metasprite) MEMBER(hitbox) MEMBER(hp) MEMBER(atk) MEMBER(collision) MEMBER(attribute) \
    MEMBER(state) MEMBER(frame_pacing)
#include <soa-struct.inc>
extern const soa::Array<ObjectInitData, (u8)ObjectType::Count> object_init_data;

namespace Objects {
    u8 load_object(ObjectType);
    u8 load_object_b2(ObjectType);
    void move_object_offscr_check(u8 slot);
    void move_object_with_solid_collision(u8 slot);
    void core_loop();
}

extern "C" XYMagnitude get_angular_speed(Speed speed, u8 angle);

extern "C" u16 distance(u8 slot1, u8 slot2);
extern "C" u8 arctan2(u8 slot1, u8 slot2);

extern const bool multidirection_lut[16];
extern const u8 direction_to_angle_lut[16];
extern const std::array<u8, 32> angle_to_direction_lut;

prg_rom_2 extern "C" u8 check_solid_collision(u8 filter, u8 obj_idx);
