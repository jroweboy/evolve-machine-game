
// This doesn't seem necessary. Consider removing it?
// enum class RoomType : u8 {
//     Root, // Starting room
//     Normal, // Average room. Maybe small chance to have a secret?
//     Leaf, // Intentionally a terminal room. Maybe always has a secret?
//     Boss, // TODO? Always has the key prize?
// };

// enum class 

#include <array>
#include <climits>
#include "map.hpp"

#ifndef NES
#include <sstream>
#include <iostream>
#include <cassert>
#else
#include <bank.h>
#include <neslib.h>
#include <nesdoug.h>
#include "rand.hpp"

#define rand Rng::rand8
#endif

constexpr u8 DUNGEON_WIDTH = 8;
constexpr u8 DUNGEON_HEIGHT = 8;
constexpr u8 DUNGEON_SIZE = DUNGEON_WIDTH * DUNGEON_HEIGHT;

// TODO maybe increase the room limit each level you beat?
constexpr u8 ROOM_LIMIT = 32;

// Offset in ram for where the sections start
constexpr u16 SectionOffset = sizeof(Room) * ROOM_LIMIT;

constexpr u8 NO_EXIT = 0xff;
constexpr u8 EXIT_PENDING = 0xfe;

// A stack allocated page of bytes defining the map structure.
using MapId = std::array<u8, DUNGEON_SIZE>;
using Exits = std::array<u8, ROOM_LIMIT>;

static constexpr u8 GetDirection(u8 me, u8 neighbor) {
    switch (me - neighbor) {
    case -1:
        return 3;
    case 1:
        return 1;
    case -16:
        return 0;
    case 16:
        return 2;
    }
}

static constexpr u8 OppositeDirection(u8 direction) {
    return (direction + 2) & 0b11;
}

#ifndef NES
u16 addr;
u8 vram[0x2000] = {};
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
    {0b0000, L"◼"},
    {0b0001, L"◀"},
    {0b0010, L"▼"},
    {0b0100, L"▲"},
    {0b1000, L"▶"},
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

static void dump_map(const MapId& map) {
    std::wstringstream first;
    std::wstringstream second;
    for (int i = 0; i < DUNGEON_HEIGHT; i++) {
        for (int j = 0; j < DUNGEON_WIDTH; j++) {
            if (map[i * DUNGEON_HEIGHT + j] == NO_EXIT) {
                first << "  ";
                second << "  ";
            } else {
                u8 id = map[i * DUNGEON_HEIGHT + j];
                u8 start = SectionOffset + id * sizeof(Section) + offsetof(Section, exit);
                u8 out = 0;
                for (u8 k = 0; k < 4; ++k) {
                    if (vram[start + k] == id) {
                        second << "b";
                    }
                    else if (vram[start + k] < DUNGEON_SIZE) {
                        out |= 1 << k;
                    }
                }
                first << LUT[out];
            }
        }
        first << std::endl;
        second << std::endl;
    }
    first << std::endl;
    second << std::endl;

    std::wcout << first.str() << std::endl;
    std::wcout << second.str() << std::endl;

}
#endif

// me is the current map index
// neighbor is one of the following
//    0
// 3 [ ] 1
//    2
static u8 GetNeighborId(u8 me, u8 direction) {
    switch (direction) {
        // The y bounds check will happen after the function by checking if its < DUNGEON_WIDTH * DUNGEON_HEIGHT
    case 0:
        return me - 16;
    case 2:
        return me + 16;
    case 1:
        if (me == DUNGEON_WIDTH - 1)
            return 0xff;
        return me + 1;
    case 3:
        if (me == 0)
            return 0xff;
        return me - 1;
    default:
        return 0xff;
    }
}

// TODO: Change vram_read/write to just vram_poke/peak for speed
// Optimized routine to read just the section's exit information and either update it 
// if its EXIT_PENDING or return false if theres no exit
static bool update_section_exit(u8 my_id, u8 direction, u8 new_exit) {
    // Read out this particular exit from the section
    auto offset = SectionOffset + my_id * sizeof(Section) + offsetof(Section, exit) + direction;
    vram_adr(offset);
    u8 exit;
    vram_read(&exit, 1);
    // Check to see if the eighbor has us marked as exit or not
    if (exit == NO_EXIT) {
        // and if we aren't an exit then don't do anything.
        return false;
    }
    
    // Since this section was waiting for us to fill this exit, update both the exit
    // and the entrance.
    vram_adr(offset);
    vram_write(&new_exit, 1);
    return true;
}

static void load_section_to_lead(u8 id) {
    vram_adr(SectionOffset + id * sizeof(Section));
    u8* cast = reinterpret_cast<u8*>(&lead);
    vram_read(cast, sizeof(Section));
}

static void load_section_to_side(u8 id) {
    vram_adr(SectionOffset + id * sizeof(Section));
    u8* cast = reinterpret_cast<u8*>(&side);
    vram_read(cast, sizeof(Section));
}

static void write_section_lead(u8 id) {
    vram_adr(SectionOffset + id * sizeof(Section));
    const u8* cast = reinterpret_cast<const u8*>(&lead);
    vram_write(cast, sizeof(Section));
}

static void write_section_side(u8 id) {
    vram_adr(SectionOffset + id * sizeof(Section));
    const u8* cast = reinterpret_cast<const u8*>(&side);
    vram_write(cast, sizeof(Section));
}

static void write_room_to_chrram(u8 id) {
    vram_adr(id * sizeof(Room));

    const u8* cast = reinterpret_cast<const u8*>(&room);
    vram_write(cast, sizeof(Room));

    // also write out the current sections
    write_section_lead(room.lead_id);
    write_section_side(room.side_id);
}

// Loads the exit data for a different room in the map
// and updates the particular exit for their neighbor
//static void update_exits(const MapId& map, u8 room_id) {
//}

// A side cell is the attached room to the new map_id, since each
// cell is 1x2 or 2x1;
static u8 get_side_cell(const MapId& map, u8 position) {
    s8 i = 3;
    u8 u = rand() & 0b11;
    while (i > 0) {
        u8 maybe_side = GetNeighborId(position, u);
        if (maybe_side < DUNGEON_SIZE && map[maybe_side] == 0xff) {
            return maybe_side;
        }
        // Check the next cell
        u = (u + 1) & 0b11;
        i--;
    }
    // Uh-oh all the side cells are full!
    return 0xff;
}

// Load up stuff into the current room
static void generate_room_spawns() {
    // TODO actually add stuff to the room
    return;
}

static void add_sections_to_fill(Section& section, MapId& to_fill, u8& fill_write, u8 me) {

    // Get a random list of exits to leave from in this first room
    u8 todo = 0;
    while (todo == 0) {
        todo = ((u8)rand()) & 0b11;
    }
    // Add all of the new exits to the todo list so we can fill them
    for (u8 i = 0; i < 4; ++i) {
        // If we didn't roll a random number that has an exit at this spot, skip it
        if ((todo & (1 << i)) == 0) continue;

        u8 map_pos = GetNeighborId(me, i);
        // If the index goes out of bounds, then skip it too
        if (map_pos >= DUNGEON_SIZE) continue;

        // otherwise, add this map position to the todo list
        to_fill[fill_write++] = map_pos;
        section.exit[i] = EXIT_PENDING;
    }
}

static void update_section_exits(Section& section, u8 me) {
    for (u8 i = 0; i < 4; ++i) {
        u8 neighbor = GetNeighborId(me, i);
        if (neighbor != EXIT_PENDING) continue;

        // if we have a neighbor that was waiting for us to add the exit, then
        // update their exits now. Get the direction *from* the neighbor to this
        // section and write that.
        u8 direction = GetDirection(neighbor, me);
        bool updated = update_section_exit(neighbor, direction, me);
        // and then if the neighbor was pending a new room, update our exit
        // with the opposite direction
        if (updated) {
            u8 opps = OppositeDirection(direction);
            section.exit[opps] = neighbor;
        }
    }
}

void generate_dungeon() {
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
    u8 side_id = NO_EXIT; // mark the side position as unused

    // now we have the initial rooms so write my id to the map
    map[lead_id] = id;
    room.lead_id = lead_id;
    room.side_id = side_id;
    lead.room_id = id;
    side.room_id = id;

    // clear out the exits for the room sections. we'll update the exits with
    // the ids to the next rooom when we process that room.
    lead.exit = {};
    side.exit = {};

    // Keep trying to add rooms to the todo list until we actually add one.
    while (fill_read == fill_write) {
        add_sections_to_fill(lead, to_fill, fill_write, lead_id);
    }

    // spawn random things into the room itself
    generate_room_spawns();

    // and then save our first room to CHR RAM.
    write_room_to_chrram(id);

    while (id < ROOM_LIMIT && fill_read < fill_write) {
        ////////////////////
        // Step 1: Find the next room's position.
        // Read the next item on the todo list and add another room.
        lead_id = to_fill[fill_read++];
        // Check that we haven't already filled this on a previous iteration.
        if (map[lead_id] != 0xff) {
            // If we did its fine to just skip it
            continue;
        }
        room.lead_id = lead_id;
        lead.room_id = id;

        ////////////////////
        // Step 2: Update the neighboring room's exits.
        // Load each of the neighbor rooms and see if what their exit looks like.
        // If its EXIT_PENDING, then its ready for us to overwrite it with our ID
        update_section_exits(lead, lead_id);

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
        bool has_side = side_id < DUNGEON_SIZE;

        if (has_side) {
            // if we have a side cell, then we should update its neighbors exits
            side.room_id = id;
            room.side_id = side_id;
            update_section_exits(side, side_id);

            // And also try to add new rooms to fill
            add_sections_to_fill(side, to_fill, fill_write, side_id);
        }

        ////////////////////
        // Step 4:

        //auto room_location = room_id * 0x100;
        //auto offset = room_location + offsetof(Room, lead_pos);

        //u8 lead_pos, side_pos;
        //vram_adr(offset);
        //vram_read(&lead_pos, 1);
        //vram_read(&side_pos, 1);


        //offset = room_location + offsetof(Room, exit);
        //std::array<u8, 8> exit;
        //std::array<u8, 8> entrance;
        //vram_adr(offset);
        //vram_read(&*exit.begin(), 8);
        //vram_read(&*entrance.begin(), 8);


        /////////////////
        //// Step 4: Generate a side room (or not randomly)
        //u8 new_side = 0xff;
        //// Give a room a 25% chance to become a single size room
        //if ((rand() & 0b11) != 0b11) {
        //    new_side = get_side_cell(map, new_main);
        //}

        //map[lead_pos] = new_lead;
        //map[side_pos] = new_side;
        //room.room_id = id;
        //room.lead_pos = new_position;
        //room.side_pos = new_side;

        ////////////////

        write_room_to_chrram(id);


        //for (u8 i = 0; i < 4; ++i) {
        //    u8 neighbor_id = map[GetIdx(position, i)];
        //    if (neighbor_id != 0 && neighbor_id != room_id) {
        //        lead_exit[i] = neighbor_id;
        //    }
        //}
        //for (u8 i = 0; i < 4; ++i) {
        //    u8 neighbor_id = map[GetIdx(position, i)];
        //    if (neighbor_id != 0 && neighbor_id != room_id) {
        //        side_exit[i] = neighbor_id;
        //    }
        //}

        //// update the exit
        //vram_adr(offset);
        //vram_write(&*lead_exit.begin(), 4);
        //vram_write(&*side_exit.begin(), 4);
        //// Try and find an opening for the next room. randomly check either the side or the main spot for available neighbors.
        //// No side spots available here, so we need to back up and try the previous slot
        //if (!has_side) {
        //    // assert(true, "test3");
        //    u8 neighbor = rand();
        //    u8& pos_or_side = (neighbor & 0b10000000) ? prev_position : prev_side;
        //    if (!(has_side = get_side_cell(map, pos_or_side, new_position))) {
        //        has_side = get_side_cell(map, (neighbor & 0b10000000) ? prev_side : prev_position, new_position);
        //    }
        //}
        //if (!has_side) {
        //    assert(true, "test1");
        //}
        // TODO: we should check to see if this ever fails. assert would be nice here
    }
#ifndef NES
    dump_map(map);
#endif
}

#ifndef NES
int main()
{
    //srand(0);
    srand(time(NULL));
    generate_dungeon();
}
#endif
