
#pragma once

#include "common.hpp"
#include "soa.h"

constexpr u8 OBJECT_COUNT = 24;

enum class ObjectType : u8 {
    Player,
    Walker,
    Count,
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
#include "soa-struct.inc"

struct Object {
    /// Offset for the metasprite to render this.
    u8 metasprite;

    /// Current status for the object. Meaning is dependent on what the object is
    /// Negative values indicate that this should not be rendered (if its a sprite)
    State state;

    /// When loading the sprite, we draw them in whatever order we find them,
    /// so the metasprite data needs to be offset by the value of the actual
    /// tile as it appears in CHR-RAM
    u8 tile_offset;

    /// Current frame of animation for this metasprite
    u8 animation_frame;

    u8 direction;
    u8 speed;

    s8 hp;
    u8 atk;
    
    s16 x;
    s16 y;
    
    Hitbox hitbox;
};

#define SOA_STRUCT Object
#define SOA_MEMBERS \
    MEMBER(metasprite) MEMBER(state) MEMBER(tile_offset) MEMBER(animation_frame) MEMBER(direction) MEMBER(speed) \
    MEMBER(hp) MEMBER(atk) MEMBER(x) MEMBER(y) MEMBER(hitbox)
#include "soa-struct.inc"

extern soa::Array<Object, OBJECT_COUNT> objects;
