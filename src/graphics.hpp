#pragma once

#include "common.hpp"
#include "header/graphics_constants.hpp"

extern "C" void donut_decompress_vram(Archive file);
extern "C" void donut_decompress_buffer(Archive file);
extern u8 decompress_buffer[64];
// extern "C" void huffmunch_load_archive(Archive file);
// extern "C" u8 huffmunch_read();
