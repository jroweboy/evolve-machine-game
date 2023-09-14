
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
    u8 metasprite;

    State state;

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
    MEMBER(metasprite) MEMBER(state) MEMBER(animation_frame) MEMBER(direction) MEMBER(speed) \
    MEMBER(hp) MEMBER(atk) MEMBER(x) MEMBER(y) MEMBER(hitbox)
#include "soa-struct.inc"

extern soa::Array<Object, OBJECT_COUNT> objects;
