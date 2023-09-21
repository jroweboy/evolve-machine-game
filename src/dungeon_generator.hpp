
#pragma once

#include "common.hpp"

namespace Dungeon {

constexpr u8 WIDTH = 8;
constexpr u8 HEIGHT = 8;
constexpr u8 SIZE = WIDTH * HEIGHT;

static constexpr u8 LeadLUT[4] = { 0x20, 0x24, 0x28, 0x20 };
static constexpr u8 SideLUT[4] = { 0x28, 0x20, 0x20, 0x24 };

// TODO maybe increase the room limit each level you beat?
constexpr u8 ROOM_LIMIT = 32;


constexpr u8 NO_EXIT = 0xff;
constexpr u8 EXIT_PENDING = 0xfe;
constexpr u8 SIDE_ROOM = 0xe0;

enum Direction {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3,
    NONE = 0xff,
};

constexpr Direction GetDirection(u8 me, u8 neighbor) {
    switch ((s8)me - (s8)neighbor) {
    case -1:
        return Direction::Left;
    case 1:
        return Direction::Right;
    case -WIDTH:
        return Direction::Up;
    case WIDTH:
        return Direction::Down;
    default:
        return Direction::NONE;
    }
}

constexpr u8 OppositeDirection(u8 direction) {
    return (direction + 2) & 0b11;
}

__attribute__((section(".prg_rom.2"))) void generate_dungeon();

void load_section_to_lead(u8 section_id);
void load_section_to_side(u8 section_id);
void write_section_lead(u8 section_id);
void write_section_side(u8 section_id);

void load_room_from_chrram(u8 room_id);
void write_room_to_chrram(u8 room_id);

// extern u8 starting_room_id;

}
