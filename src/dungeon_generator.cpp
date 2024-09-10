
// This doesn't seem necessary. Consider removing it?
// enum class RoomType : u8 {
//     Root, // Starting room
//     Normal, // Average room. Maybe small chance to have a secret?
//     Leaf, // Intentionally a terminal room. Maybe always has a secret?
//     Boss, // TODO? Always has the key prize?
// };

// enum class 

#include <array>
#include <fixed_point.h>

#include "dungeon_generator.hpp"
#include "common.hpp"
#include "map.hpp"
#include "object.hpp"
//#include "object.hpp"

// Offset in ram for where the sections start
constexpr u16 SectionOffset = sizeof(Room) * Dungeon::ROOM_LIMIT;

// A stack allocated page of bytes defining the map structure.
using MapId = std::array<u8, Dungeon::SIZE>;
using Exits = std::array<u8, Dungeon::ROOM_LIMIT>;

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
    for (int i = 0; i < Dungeon::HEIGHT; i++) {
        std::wstringstream second;
        for (int j = 0; j < Dungeon::WIDTH; j++) {
            u8 id = i * Dungeon::HEIGHT + j;
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
                    u8 exit_direction = vram[start + k];
                    if (exit_direction == Dungeon::SIDE_ROOM) {
                        has_side = true;
                        out |= 1 << k;
                    }
                    if ((exit_direction & 0x80) == 0) {
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

// Optimized routine to read just the section's exit information and either update it 
// if its Dungeon::EXIT_PENDING or return false if theres no exit
prg_rom_2 static u8 update_section_exit(u8 my_id, u8 direction, u8 new_exit) {
    // Read out this particular exit from the section
    u16 offset = SectionOffset + my_id * sizeof(Section) + offsetof(Section, exit) + direction;
    vram_adr(offset);
    PEEK(0x2007);
    u8 exit = PEEK(0x2007);
    // vram_read(&exit, 1);
    // Check to see if the neighbor has us marked as exit or not
    if (exit == Dungeon::NO_EXIT || exit == Dungeon::SIDE_ROOM) {
        // and if we aren't an exit then don't do anything.
        return exit;
    }
    
    // Since this section was waiting for us to fill this exit, update both the exit
    // and the entrance.
    vram_adr(offset);
    POKE(0x2007, new_exit);
    // vram_write(&new_exit, 1);
    return exit;
}

// A side cell is the attached room to the new map_id, since each
// cell is 1x2 or 2x1;
prg_rom_2 static u8 get_side_cell(const MapId& map, u8 position) {
    s8 i = 3;
    u8 u = rand() & 0b11;
    while (i >= 0) {
        u8 maybe_side = Dungeon::GetNeighborId(position, u);
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
        if ((todo & (1 << i)) == 0 || section.exit[i] <= Dungeon::SIDE_ROOM) continue;

        u8 map_pos = Dungeon::GetNeighborId(me, i);
        // If the index goes out of bounds, then skip it too
        if (map_pos >= Dungeon::SIZE) continue;

        // otherwise, add this map position to the todo list
        to_fill[fill_write++] = map_pos;
        section.exit[i] = Dungeon::EXIT_PENDING;
    }
}

prg_rom_2 static void update_section_exits(const MapId& map, Section& section, u8 me) {
    for (u8 i = 0; i < 4; ++i) {
        u8 neighbor = Dungeon::GetNeighborId(me, i);
        if (neighbor >= Dungeon::SIZE || map[neighbor] == Dungeon::NO_EXIT) continue;
        // if we have a neighbor that was waiting for us to add the exit, then
        // update their exits now. Get the direction *from* me to the neighbor
        // section and write that.
        u8 direction = Dungeon::GetDirection(me, neighbor);
        u8 opps = Dungeon::OppositeDirection(direction);
        u8 updated_exit = update_section_exit(neighbor, direction, opps);
        if (updated_exit == Dungeon::NO_EXIT) {
            // if the neighbor has no_exit marked here, then mark this as no exit too
            section.exit[direction] = Dungeon::NO_EXIT;
            continue;
        } else if (updated_exit == Dungeon::SIDE_ROOM) {
            section.exit[opps] = Dungeon::SIDE_ROOM;
        } else {
            // and then if the neighbor was pending a new room, update our exit
            // with the opposite direction
            section.exit[opps] = direction;
        }
    }
}

namespace Dungeon {

// me is the current map index
// neighbor is one of the following
//    0
// 3 [ ] 1
//    2
prg_rom_2 u8 GetNeighborId(u8 me, u8 direction) {
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
        // DEBUGGER(direction);
        return 0xff;
    }
}

// u8 starting_room_id;

prg_rom_2 u8 load_room_id_by_section(u8 id) {
    vram_adr(SectionOffset + (u16)id * sizeof(Section) + offsetof(Section, room_id));
    PEEK(0x2007);
    return PEEK(0x2007);
    // vram_read(&room_id, sizeof(u8));
    // return room_id;
}

prg_rom_2 void load_section_to_lead(u8 id) {
    vram_adr(SectionOffset + (u16)id * sizeof(Section));
    u8* cast = reinterpret_cast<u8*>(&lead);
    vram_read(cast, sizeof(Section));
}

prg_rom_2 void load_section_to_side(u8 id) {
    if (id == Dungeon::NO_EXIT) {
        side.room_id = Dungeon::NO_EXIT;
        return;
    }
    vram_adr(SectionOffset + (u16)id * sizeof(Section));
    u8* cast = reinterpret_cast<u8*>(&side);
    vram_read(cast, sizeof(Section));
}

prg_rom_2 void write_section_lead(u8 id) {
    vram_adr(SectionOffset + (u16)id * sizeof(Section));
    const u8* cast = reinterpret_cast<const u8*>(&lead);
    vram_write(cast, sizeof(Section));
}

prg_rom_2 void write_section_side(u8 id) {
    vram_adr(SectionOffset + (u16)id * sizeof(Section));
    const u8* cast = reinterpret_cast<const u8*>(&side);
    vram_write(cast, sizeof(Section));
}

prg_rom_2 void load_room_from_chrram(u8 id) {
    vram_adr(id * sizeof(Room));
    u8* cast = reinterpret_cast<u8*>(&room);
    vram_read(cast, sizeof(Room));
}

prg_rom_2 void write_room_to_chrram(u8 id) {
    vram_adr(id * sizeof(Room));

    const u8* cast = reinterpret_cast<const u8*>(&room);
    vram_write(cast, sizeof(Room));
}

prg_rom_2 static void set_room_xy(u8 map_id) {
    // Side is the top room, so the X,Y coords are based on that one.
    room.x = ((u16)(map_id & 0b111)) << 8;
    room.y = ((u16)(map_id / 8)) << 8;
}

prg_rom_2 GenerateStats generate_dungeon() {
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

    vram_adr(0);
    vram_fill(0xff, 0x2000);
#endif

    ////////////////////
    // Step 0: Manually build out the root room.
    u8 id = 0;

    // set the main to a random location with a vertical side room
    u8 lead_id;
    // room a room that is not along the top of the screen.
    while ((lead_id = ((u8)rand()) & 0b111111) < Dungeon::WIDTH);
    u8 start_id = lead_id;
    u8 side_id = GetNeighborId(lead_id, Direction::Up);

    // now we have the initial rooms so write my id to the map
    map[lead_id] = id;
    map[side_id] = id;
    // starting_room_id = lead_id;
    room.lead_id = lead_id;
    room.side_id = side_id;
    room.scroll = ScrollType::Vertical;
    lead.room_id = id;
    lead.nametable = 0x28;
    lead.room_base = SectionBase::StartDown;
    side.room_id = id;
    side.nametable = 0x20;
    side.room_base = SectionBase::StartUp;
    // room x/y is based on the top left spot on the map
    set_room_xy(side_id);
    
    ////////////////////
    // Step -1: Set the player position for debug purposes
    objects[0].x = f16_8(room.x);
    objects[0].y = f16_8(room.y);

    // clear out the exits for the room sections. we'll update the exits with
    // the ids to the next rooom when we process that room.
    for (auto& num : lead.exit) { num = NO_EXIT; }
    for (auto& num : side.exit) { num = NO_EXIT; }

    // Manually connect the starting rooms together to prevent the following code from trying to generate rooms over them
    lead.exit[Direction::Up] = Dungeon::SIDE_ROOM;
    side.exit[Direction::Down] = Dungeon::SIDE_ROOM;

    // Keep trying to add rooms to the todo list until we actually add one.
    while (fill_read == fill_write) {
        add_sections_to_fill(lead, to_fill, fill_write, lead_id);
    }

    // We need to hard code the item spawns in the room.
    // spawn random things into the room itself
    
    // start the player at the bottom center of the screen
    lead.objects[0] = {ObjectType::Player, Normal, (s16)(room.x + 100), (s16)(room.y + 240 + 100)};
    lead.objects[1] = {ObjectType::WeaponCube, Normal, (s16)(room.x + 25), (s16)(room.y + 240 + 100)};
    lead.objects[2] = {ObjectType::WeaponDiamond, Normal, (s16)(room.x + 50),  (s16)(room.y + 240 + 100)};
    lead.objects[3] = {ObjectType::WeaponPyramid, Normal, (s16)(room.x + 125), (s16)(room.y + 240 + 100)};
    lead.objects[4] = {ObjectType::WeaponSphere, Normal, (s16)(room.x + 150), (s16)(room.y + 240 + 100)};

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

    // for (u8 k=0; k < 30; k++) {
    //     ppu_wait_nmi();
    // }

    while ((++id) < ROOM_LIMIT && fill_read < fill_write) {
        ////////////////////
        // Step 1: Find the next room's position.
        // Read the next item on the todo list and add another room.
        lead_id = to_fill[fill_read++];
        // Check that we haven't already filled this on a previous iteration.
        if (map[lead_id] != NO_EXIT) {
            // If we did its fine to just skip it
            continue;
        }
        map[lead_id] = id;
        room.lead_id = lead_id;
        lead.room_id = id;
        for (auto& num : lead.exit) { num = NO_EXIT; }
        // Default the scroll type to single and update it later if we add a side room.
        room.scroll = ScrollType::Single;

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
            // Direction dir = GetDirection(side_id, lead_id);
            Direction dir = GetDirection(lead_id, side_id);
            // DEBUGGER(dir);
            lead.nametable = LeadLUT[dir];
            side.nametable = SideLUT[dir];
            // side.nametable = LeadLUT[dir];
            // lead.nametable = SideLUT[dir];
            switch (dir) {
                case Direction::Up:
                    lead.room_base = SectionBase::Top;
                    side.room_base = SectionBase::Bottom;
                    // lead.room_base = SectionBase::Bottom;
                    // side.room_base = SectionBase::Top;
                    room.scroll = ScrollType::Vertical;
                    set_room_xy(lead_id);
                    // set_room_xy(side_id);
                    break;
                case Direction::Down:
                    lead.room_base = SectionBase::Bottom;
                    side.room_base = SectionBase::Top;
                    // lead.room_base = SectionBase::Top;
                    // side.room_base = SectionBase::Bottom;
                    room.scroll = ScrollType::Vertical;
                    set_room_xy(side_id);
                    // set_room_xy(lead_id);
                    break;
                case Direction::Right:
                    lead.room_base = SectionBase::Right;
                    side.room_base = SectionBase::Left;
                    // lead.room_base = SectionBase::Left;
                    // side.room_base = SectionBase::Right;
                    room.scroll = ScrollType::Horizontal;
                    set_room_xy(side_id);
                    // set_room_xy(lead_id);
                    break;
                case Direction::Left:
                    lead.room_base = SectionBase::Left;
                    side.room_base = SectionBase::Right;
                    // lead.room_base = SectionBase::Right;
                    // side.room_base = SectionBase::Left;
                    room.scroll = ScrollType::Horizontal;
                    set_room_xy(lead_id);
                    // set_room_xy(side_id);
                    break;
                default:
                    break;
            }
            
            // Since we know we have a side and we know the direction, update this info here
            // lead.exit[dir] = Dungeon::SIDE_ROOM;
            lead.exit[OppositeDirection(dir)] = Dungeon::SIDE_ROOM;
        }
        else {
            lead.nametable = 0x20;
            lead.room_base = SectionBase::Single;
            // room.scroll = ScrollType::Single;
            set_room_xy(lead_id);
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
            for (auto& num : side.exit) { num = NO_EXIT; }

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

        // for (u8 k=0; k < 30; k++) {
        //     ppu_wait_nmi();
        // }

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

    return GenerateStats{.start_id = start_id, .room_count = id};
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
    Dungeon::generate_dungeon();
}
#endif
