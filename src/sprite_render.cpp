
// #include <mapper.h>
// #include <neslib.h>
// #include <soa.h>

// #include "map.hpp"
// #include "sprite_render.hpp"
#include "common.hpp"
// #include "object.hpp"

struct OAMData {
    u8 y;
    u8 tile;
    u8 attr;
    u8 x;
};


__attribute__((section(".zp"))) u8 view_x;
__attribute__((section(".zp"))) u8 view_y;

noinit u8 shuffle_offset;
noinit u8 sprite_slot;

extern u8 OAM_BUF[256];
