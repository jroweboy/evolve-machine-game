
#pragma once

#include "common.hpp"

/// Tracks the current offset to write the BG CHR tiles to for the offset.
/// When adding the CHR tiles, this is a convenient way to know what the last VRAM address is
/// so we can draw the next CHR tiles here and move it forward after.
extern "C" u16 bg_chr_offset;
/// When generating the dungeon map, we want to make sure we don't spawn things that
/// over flow the chr count, so this is used in each room to make sure that its within
/// that parameter. This is also written to the object data itself, so that the metasprite rendering
/// code can offset the tile in the metasprite by the location it is in CHR-RAM
extern "C" u8 bg_chr_count;

/// Same as `bg_chr_offset` but for the CHR in 0x1000
extern "C" u16 sp_chr_offset;
extern "C" u8 sp_chr_count;

extern "C" u8 hud_tile_offset;


extern "C" u8 object_tile_offset[];

namespace MapLoader {
    void load_map(u8 room_id);
}
