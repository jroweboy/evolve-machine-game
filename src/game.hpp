
#pragma once

#include "common.hpp"

extern "C" u16 room_bounds_x;
extern "C" u16 room_bounds_y;

namespace Game {
    prg_rom_2 void init();
    prg_rom_2 void update();
}