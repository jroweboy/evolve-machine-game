
#include <array>

#include <bank.h>
#include <cstddef>
#include <nesdoug.h>
#include <neslib.h>
#include <peekpoke.h>

#include "chr.hpp"
#include "map_loader.hpp"
#include "map.hpp"
#include "music.hpp"
#include "rand.hpp"

extern const char test_map_bin;

Room room;

constexpr u8 DUNGEON_WIDTH = 16;
constexpr u8 DUNGEON_HEIGHT = 16;

// A stack allocated page of bytes defining the map structure.
using MapId = std::array<u8, DUNGEON_WIDTH * DUNGEON_HEIGHT>;

// TODO maybe increase the room limit each level you beat?
constexpr u8 ROOM_LIMIT = 32;

// me is the current map index
// neighbor is one of the following
//    0
// 3 [ ] 1
//    2
static u8 GetIdx(u8 me, u8 neighbor) {
    s8 x_off = (neighbor == 1) ? 1 : (neighbor == 3) ? -1 : 0;
    s8 y_off = (neighbor == 0) ? -16 : (neighbor == 2) ? 16 : 0;
    return me + x_off + y_off;
}

static void write_room_to_chrram(u8 id, const Room* room) {
    // Just align them to 256 bytes for now
    vram_adr((id - 1) * 0x100);

    const u8* cast = reinterpret_cast<const u8*>(room);
    vram_write(cast, sizeof(Room));
}

// Loads the exit data for a different room in the map
// and updates the particular exit for their neighbor
static void update_exits(const MapId& map, u8 room_id) {
    auto room_location = (room_id-1) * 0x100;
    auto offset = room_location + offsetof(Room, position);

    u8 position, buddy;
    vram_adr(offset);
    vram_read(&position, 1);
    vram_read(&buddy, 1);
    
    
    offset = room_location + offsetof(Room, main_exit);
    std::array<u8, 4> main_exit;
    std::array<u8, 4> buddy_exit;
    vram_adr(offset);
    vram_read(main_exit.begin(), 4);
    vram_read(buddy_exit.begin(), 4);

    for (u8 i = 0; i < 4; ++i) {
        u8 neighbor_id = map[GetIdx(position, i)];
        if (neighbor_id != 0 && neighbor_id != room_id) {
            main_exit[i] = neighbor_id;
        }
    }
    for (u8 i = 0; i < 4; ++i) {
        u8 neighbor_id = map[GetIdx(position, i)];
        if (neighbor_id != 0 && neighbor_id != room_id) {
            buddy_exit[i] = neighbor_id;
        }
    }

    // update the exit
    vram_adr(offset);
    vram_write(main_exit.begin(), 4);
    vram_write(buddy_exit.begin(), 4);
}

// A buddy cell is the attached room to the new map_id, since each
// cell is 1x2 or 2x1;
static bool get_buddy_cell(MapId& map, u8 position, u8& buddy) {
    s8 i = 3;
    u8 u = rand8() & 0b11;
    while (i > 0) {
        u8 maybe_buddy = GetIdx(position, u);
        if (map[maybe_buddy] == 0xff) {
            buddy = maybe_buddy;
            return true;
        }
        // Check the next cell
        u = (u + 1) % 4;
        i--;
    }
    // Uh-oh all the buddy cells are full!
    return false;
}

// Load up stuff into the current room
static void generate_room_spawns() {
    // TODO actually add stuff to the room
    return;
}

static void generate_random_walk() {
    // List of visited tiles and their room_ids
    MapId map;
    for (auto& num : map) { num = 0xff; }

    // Setup the initial bank
    set_chr_bank(3);

    ///////////////////
    // Step 0: Manually build out the root room.
    u8 id = 1;

    // First find a position to put the buddy at. Since its the root
    // we are guaranteed to find the buddy the first try.
    u8 position, buddy;
    bool has_buddy = false;
    while (!has_buddy) {
        position = Rng::rand8();
        has_buddy = get_buddy_cell(map, position, buddy);
    }

    // now we have the initial rooms so write my id to the map
    map[position] = id;
    map[buddy] = id;
    room.room_id = id;
    room.position = position;
    room.buddy = buddy;

    // Store the previous room in case we need to back track.
    // TODO: if we get really lost, it might run out of previous rooms?
    // maybe build this algorithm on PC and test it without the NES in the way
    u8 prev_position = position;
    u8 prev_buddy = buddy;

    // spawn random things into the room itself
    generate_room_spawns();

    // clear out the exits for the room. we'll update them later when we make a new walk
    room.main_exit = {};
    room.buddy_exit = {};
    // and then save our first room to CHR RAM.
    write_room_to_chrram(id, &room);

    for (id=2; id <= ROOM_LIMIT; ++id) {
        /////////////////
        // Step 1: Find the next room's position.
        // Try all the exits of the previous room's position/buddy (pick pos or buddy at random)
        // and if it fails, try all previous room's position/buddy exits

        // Try and find an opening for the next room. randomly check either the buddy or the main spot for available neighbors.
        u8 neighbor = rand8();
        u8 new_position, new_buddy;
        u8& pos_or_buddy = (neighbor & 0b10000000) ? position : buddy;
        if (!(has_buddy = get_buddy_cell(map, pos_or_buddy, new_position))) {
            has_buddy = get_buddy_cell(map, (neighbor & 0b10000000) ? buddy : position, new_position);
        }
        // No buddy spots available here, so we need to back up and try the previous slot
        if (!has_buddy) {
            // assert(true, "test3");
            pos_or_buddy = (neighbor & 0b10000000) ? prev_position : prev_buddy;
            if (!(has_buddy = get_buddy_cell(map, pos_or_buddy, new_position))) {
                has_buddy = get_buddy_cell(map, (neighbor & 0b10000000) ? prev_buddy : prev_position, new_position);
            }
        }
        //if (!has_buddy) {
        //    assert(true, "test1");
        //}
        // TODO: we should check to see if this ever fails. assert would be nice here

        // Now that we have our new main position, try to find a buddy
        if (!(has_buddy = get_buddy_cell(map, new_position, new_buddy))) {
            // TODO: we should check to see if this ever fails. assert would be nice here
            // if this fails maybe we should jump back to the first tile?
            // assert(true, "test2");
        }

        // now we know what the next exit is, so lets update the previous room exit
        update_exits(map, id - 1);

        prev_position = position;
        prev_buddy = buddy;
        position = new_position;
        buddy = new_buddy;

        map[position] = id;
        map[buddy] = id;
        room.room_id = id;
        room.position = new_position;
        room.buddy = new_buddy;

        // Add exits to all of the rooms that we are touching on the map.
        room.main_exit = {};
        room.buddy_exit = {};
        write_room_to_chrram(id, &room);
        // a bit lame, but to save code space, call update exits on the room we just wrote
        update_exits(map, id);
    }
}

namespace MapLoader {

    void update() {}

    void init() {
        // TODO turn off DPCM if its playing

        constexpr char background_pal[] = {
            0x0f, 0x16, 0x27, 0x2d,
            0x0f, 0x1c, 0x31, 0x30,
            0x0f, 0x10, 0x20, 0x30,
            0x0f, 0x10, 0x20, 0x30,
        };

        ppu_off();

        generate_random_walk();

        // switch to the 16kb bank that holds the level
        set_prg_bank(1);
        
        set_chr_bank(0);
        vram_adr(0x00);
        donut_decompress(&test_map_bin);

        vram_adr(NAMETABLE_A);
        vram_unrle(CHR::test_bg_nametable1);
        vram_adr(NAMETABLE_B);
        vram_unrle(CHR::test_bg_nametable2);

        pal_bg(background_pal);
        pal_bright(0);

        scroll(0, 0);
        ppu_on_all();
        pal_fade_to(0, 4);

        play_song(Song::TitleAmbience);
    }
}
