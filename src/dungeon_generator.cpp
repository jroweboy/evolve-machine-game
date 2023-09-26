﻿
// This doesn't seem necessary. Consider removing it?
// enum class RoomType : u8 {
//     Root, // Starting room
//     Normal, // Average room. Maybe small chance to have a secret?
//     Leaf, // Intentionally a terminal room. Maybe always has a secret?
//     Boss, // TODO? Always has the key prize?
// };

// enum class 

#include <array>

#include "dungeon_generator.hpp"
#include "map.hpp"

#ifndef NES
#include <sstream>
#include <iostream>
#include <cassert>
#else
#include <mapper.h>
#include <neslib.h>
#include "rand.hpp"

#define rand Rng::rand8
#endif

#ifndef NES
u16 addr;
u8 vram[0x2000] = {};
void init() {
    for (int i = 0; i < 0x2000; ++i) vram[i] = 0xff;
}
Room room;
Section lead;
Section side;

static void vram_adr(u16 a) { addr = a; }
static void vram_read(u8* vals, int len) { for (int i = 0; i < len; ++i) { vals[i] = vram[addr++]; } }
static void vram_write(const u8* vals, int len) { for (int i = 0; i < len; ++i) { vram[addr++] = vals[i]; } }

//    0
// 3 [ ] 1
//    2
#include <unordered_map>
static std::unordered_map<u8, std::wstring> LUT { 
    {0b0000, L"╳"},
    {0b0001, L"▽"},
    {0b0010, L"◁"},
    {0b0100, L"△"},
    {0b1000, L"▷"},
    {0b0011, L"╚"},
    {0b0101, L"║"},
    {0b1001, L"╝"},
    {0b0110, L"╔"},
    {0b1010, L"═"},
    {0b1100, L"╗"},
    {0b0111, L"╠"},
    {0b1011, L"╩"},
    {0b1101, L"╣"},
    {0b1110, L"╦"},
    {0b1111, L"╬"},
};

static void dump_map(const MapId& map, const MapId& to_fill) {
    std::wstringstream first;
    for (int i = 0; i < DUNGEON_HEIGHT; i++) {
        std::wstringstream second;
        for (int j = 0; j < DUNGEON_WIDTH; j++) {
            u8 id = i * DUNGEON_HEIGHT + j;
            auto filling = std::find(to_fill.begin(), to_fill.end(), id);
            if (map[id] == Dungeon::NO_EXIT) {
                first << ((filling != to_fill.end()) ? L"▧" : L"·");
                second << L"·";
            } else {
                u16 start = SectionOffset + (u16)id * sizeof(Section) + offsetof(Section, exit);
                u8 out = 0;

                bool has_side = false;
                bool has_unmatched = false;
                for (u8 k = 0; k < 4; ++k) {
                    u8 exit_id = vram[start + k];
                    if (exit_id < Dungeon::SIZE) {
                        if (map[exit_id] == map[id]) {
                            has_side = true;
                        }
                        out |= 1 << k;
                    }
                    //if (exit_id == Dungeon::EXIT_PENDING) {
                    //    has_unmatched = true;
                    //}
                }
                if (map[id] == 0) {
                    out = 0; // Keep the start location drawn with an X
                }
                first << ((has_unmatched) ? L"◳" : LUT[out]);
                second << ((has_side) ? L'b' : L'a');
            }
        }
        first << "  " << second.str() << std::endl;
    }
    first << std::endl;
    //second << std::endl;

    std::wcout << first.str() << std::endl;
    //std::wcout << second.str() << std::endl;

}
#endif

// Offset in ram for where the sections start
constexpr u16 SectionOffset = sizeof(Room) * Dungeon::ROOM_LIMIT;

// A stack allocated page of bytes defining the map structure.
using MapId = std::array<u8, Dungeon::SIZE>;
using Exits = std::array<u8, Dungeon::ROOM_LIMIT>;

// me is the current map index
// neighbor is one of the following
//    0
// 3 [ ] 1
//    2
prg_rom_2 static u8 GetNeighborId(u8 me, u8 direction) {
    switch (direction) {
        // The y bounds check will happen after the function by checking if its < DUNGEON_WIDTH * DUNGEON_HEIGHT
    case 0:
        return me - Dungeon::WIDTH;
    case 2:
        return me + Dungeon::WIDTH;
    case 1:
        if ((me & 0b111) == Dungeon::WIDTH - 1)
            return 0xff;
        return me + 1;
    case 3:
        if ((me & 0b111) == 0)
            return 0xff;
        return me - 1;
    default:
        return 0xff;
    }
}

// TODO: Change vram_read/write to just vram_poke/peak for speed
// Optimized routine to read just the section's exit information and either update it 
// if its Dungeon::EXIT_PENDING or return false if theres no exit
prg_rom_2 static bool update_section_exit(u8 my_id, u8 direction, u8 new_exit) {
    // Read out this particular exit from the section
    u16 offset = SectionOffset + my_id * sizeof(Section) + offsetof(Section, exit) + direction;
    vram_adr(offset);
    u8 exit;
    vram_read(&exit, 1);
    // Check to see if the eighbor has us marked as exit or not
    if (exit == Dungeon::NO_EXIT) {
        // and if we aren't an exit then don't do anything.
        return false;
    }
    
    // Since this section was waiting for us to fill this exit, update both the exit
    // and the entrance.
    vram_adr(offset);
    vram_write(&new_exit, 1);
    return true;
}

// Loads the exit data for a different room in the map
// and updates the particular exit for their neighbor
//static void update_exits(const MapId& map, u8 room_id) {
//}

// A side cell is the attached room to the new map_id, since each
// cell is 1x2 or 2x1;
prg_rom_2 static u8 get_side_cell(const MapId& map, u8 position) {
    s8 i = 3;
    u8 u = rand() & 0b11;
    while (i > 0) {
        u8 maybe_side = GetNeighborId(position, u);
        if (maybe_side < Dungeon::SIZE && map[maybe_side] == 0xff) {
            return maybe_side;
        }
        // Check the next cell
        u = (u + 1) & 0b11;
        i--;
    }
    // Uh-oh all the side cells are full!
    return Dungeon::NO_EXIT;
}

// Load up stuff into the current room
prg_rom_2 static void generate_room_spawns() {
    // TODO actually add stuff to the room
    return;
}

prg_rom_2 static void add_sections_to_fill(Section& section, MapId& to_fill, u8& fill_write, u8 me) {

    // Get a random list of exits to leave from in this first room
    u8 todo = ((u8)rand()) & 0b1111;
    // Add all of the new exits to the todo list so we can fill them
    for (u8 i = 0; i < 4; ++i) {
        // If we didn't roll a random number that has an exit at this spot, skip it
        // if we rolled a side that already has an exit skip it.
        if ((todo & (1 << i)) == 0 || section.exit[i] < Dungeon::SIZE) continue;

        u8 map_pos = GetNeighborId(me, i);
        // If the index goes out of bounds, then skip it too
        if (map_pos >= Dungeon::SIZE) continue;

        // otherwise, add this map position to the todo list
        to_fill[fill_write++] = map_pos;
        section.exit[i] = Dungeon::EXIT_PENDING;
    }
}

prg_rom_2 static void update_section_exits(const MapId& map, Section& section, u8 me) {
    for (u8 i = 0; i < 4; ++i) {
        u8 neighbor = GetNeighborId(me, i);
        if (neighbor >= Dungeon::SIZE || map[neighbor] == Dungeon::NO_EXIT) continue;

        // if we have a neighbor that was waiting for us to add the exit, then
        // update their exits now. Get the direction *from* me to the neighbor
        // section and write that.
        u8 direction = Dungeon::GetDirection(me, neighbor);
        bool updated = update_section_exit(neighbor, direction, me);
        // and then if the neighbor was pending a new room, update our exit
        // with the opposite direction
        if (updated) {
            u8 opps = Dungeon::OppositeDirection(direction);
            section.exit[opps] = neighbor;
        }
    }
}

namespace Dungeon {

// u8 starting_room_id;

u8 load_room_id_by_section(u8 id) {
    vram_adr(SectionOffset + id * sizeof(Section));
    u8 room_id;
    vram_read(&room_id, sizeof(u8));
    return room_id;
}

void load_section_to_lead(u8 id) {
    vram_adr(SectionOffset + id * sizeof(Section));
    u8* cast = reinterpret_cast<u8*>(&lead);
    vram_read(cast, sizeof(Section));
}

void load_section_to_side(u8 id) {
    if (id == Dungeon::NO_EXIT) {
        side.room_id = Dungeon::NO_EXIT;
        return;
    }
    vram_adr(SectionOffset + id * sizeof(Section));
    u8* cast = reinterpret_cast<u8*>(&side);
    vram_read(cast, sizeof(Section));
}

void write_section_lead(u8 id) {
    vram_adr(SectionOffset + id * sizeof(Section));
    const u8* cast = reinterpret_cast<const u8*>(&lead);
    vram_write(cast, sizeof(Section));
}

void write_section_side(u8 id) {
    vram_adr(SectionOffset + id * sizeof(Section));
    const u8* cast = reinterpret_cast<const u8*>(&side);
    vram_write(cast, sizeof(Section));
}

void load_room_from_chrram(u8 id) {
    vram_adr(id * sizeof(Room));
    u8* cast = reinterpret_cast<u8*>(&room);
    vram_read(cast, sizeof(Room));
}

void write_room_to_chrram(u8 id) {
    vram_adr(id * sizeof(Room));

    const u8* cast = reinterpret_cast<const u8*>(&room);
    vram_write(cast, sizeof(Room));
}

prg_rom_2 u8 generate_dungeon() {
    // List of visited tiles and their room_ids
    MapId map;
    for (auto& num : map) { num = 0xff; }

    MapId to_fill;
    u8 fill_read = 0;
    u8 fill_write = 0;
    for (auto& num : to_fill) { num = 0xff; }

#ifdef NES
    ppu_off();
    
    // Setup the initial bank
    set_chr_bank(3);
#endif

    ////////////////////
    // Step 0: Manually build out the root room.
    u8 id = 0;

    // set the main to a random location with no side room.
    u8 lead_id = ((u8)rand()) & 0b111111;
    u8 start_id = lead_id;
    u8 side_id = Dungeon::NO_EXIT; // mark the side position as unused

    // now we have the initial rooms so write my id to the map
    map[lead_id] = id;
    // starting_room_id = lead_id;
    room.lead_id = lead_id;
    room.side_id = side_id;
    lead.room_id = id;
    lead.nametable = 0x20;
    lead.room_base = SectionBase::Start;
    side.room_id = id;

    // clear out the exits for the room sections. we'll update the exits with
    // the ids to the next rooom when we process that room.
    for (auto& num : lead.exit) { num = 0xff; }
    for (auto& num : side.exit) { num = 0xff; }

    // Keep trying to add rooms to the todo list until we actually add one.
    while (fill_read == fill_write) {
        add_sections_to_fill(lead, to_fill, fill_write, lead_id);
    }

    // We need to hard code the item spawns in the room.
    // spawn random things into the room itself
    // generate_room_spawns();

    // and then save our first room to CHR RAM.
    write_room_to_chrram(id);

    // also write out the current sections
    write_section_lead(room.lead_id);
    write_section_side(room.side_id);
#ifndef NES
    std::wcout << L"id: " << id << std::endl;
    std::wcout << L"fill_read: " << fill_read << std::endl;
    std::wcout << L"fill_write: " << fill_write << std::endl;
    dump_map(map, to_fill);
#endif

    while ((++id) < ROOM_LIMIT && fill_read < fill_write) {
        ////////////////////
        // Step 1: Find the next room's position.
        // Read the next item on the todo list and add another room.
        lead_id = to_fill[fill_read++];
        // Check that we haven't already filled this on a previous iteration.
        if (map[lead_id] != 0xff) {
            // If we did its fine to just skip it
            continue;
        }
        map[lead_id] = id;
        room.lead_id = lead_id;
        lead.room_id = id;
        for (auto& num : lead.exit) { num = 0xff; }

        ////////////////////
        // Step 2: Update the neighboring room's exits.
        // Load each of the neighbor rooms and see if what their exit looks like.
        // If its Dungeon::EXIT_PENDING, then its ready for us to overwrite it with our ID
        update_section_exits(map, lead, lead_id);

        ////////////////////
        // Step 3: Randomly add sections to fill (try for at least one)
        // This will give us future rooms to try and build out later
        add_sections_to_fill(lead, to_fill, fill_write, lead_id);

        ////////////////////
        // Step 3: Determine if we are going to have a side room, and if we do, set it up now
        // There's a 75% chance that we roll a side room
        side_id = 0xff;
        if (((u8)rand() & 0b11) != 0) {
            // RNG determines we should try to find a side room.
            side_id = get_side_cell(map, lead_id);
        }

        bool has_side = side_id < Dungeon::SIZE;

        ///////////////////
        // Step 4: Determine which room goes in what nametable each room belongs
        // The map drawing routine needs to know what nametable is when laying it out
        // so that it can pick the right art base for it.

        if (has_side) {
            // Lookup table to get the nametable from the direction.
            // I did the GetDirection backwards so it lines up that the
            // lead will be up and left in this setup. (So if direction is UP,
            // then the lead is the first nametable.)
            Direction dir = GetDirection(side_id, lead_id);
            lead.nametable = LeadLUT[dir];
            side.nametable = SideLUT[dir];
            switch (dir) {
                case Direction::Up:
                    lead.room_base = SectionBase::Top;
                    side.room_base = SectionBase::Bottom;
                    break;
                case Direction::Down:
                    lead.room_base = SectionBase::Bottom;
                    side.room_base = SectionBase::Top;
                    break;
                case Direction::Right:
                    lead.room_base = SectionBase::Right;
                    side.room_base = SectionBase::Left;
                    break;
                case Direction::Left:
                    lead.room_base = SectionBase::Left;
                    side.room_base = SectionBase::Right;
                    break;
                default:
                    break;
            }
            
            // Since we know we have a side and we know the direction, update this info here
            lead.exit[dir] = side_id;
        }
        else {
            lead.nametable = 0x20;
            lead.room_base = SectionBase::Single;
        }


        ////////////////////
        // Step 5: Generate spawns for both the lead and side rooms
        generate_room_spawns();

        // write out the lead section. we need to do this before the side because
        // we will update the neighbor afterwards
        write_section_lead(room.lead_id);

        if (has_side) {
            // if we have a side cell, then we should update its neighbors exits
            map[side_id] = id;
            room.side_id = side_id;
            side.room_id = id;
            for (auto& num : side.exit) { num = 0xff; }

            update_section_exits(map, side, side_id);

            // And also try to add new rooms to fill
            add_sections_to_fill(side, to_fill, fill_write, side_id);
        }

        ////////////////
        // Step 6: Save them to disk

        if (has_side) {
            write_section_side(room.side_id);
        }

        write_room_to_chrram(id);

#ifndef NES
        std::wcout << L"id: " << id << std::endl;
        std::wcout << L"fill_read: " << fill_read << std::endl;
        std::wcout << L"fill_write: " << fill_write << std::endl;
        dump_map(map, to_fill);
#endif
    }
#ifndef NES
    std::wcout << L"id: " << id << std::endl;
    std::wcout << L"fill_read: " << fill_read << std::endl;
    std::wcout << L"fill_write: " << fill_write << std::endl;
    dump_map(map, to_fill);
#endif

    return start_id;
}
}
#ifndef NES
#include <io.h>
#include <fcntl.h>
int main()
{

    init();
    srand(time(NULL));
    //srand(0);
    //system("chcp 65001");
    _setmode(_fileno(stdout), _O_U16TEXT);
    generate_dungeon();
}
#endif
