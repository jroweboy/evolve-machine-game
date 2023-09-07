
#include <array>

#include <bank.h>
#include <cstddef>
#include <nesdoug.h>
#include <neslib.h>
#include <peekpoke.h>

#include "chr.hpp"
#include "map.hpp"
#include "music.hpp"
#include "generate_seed.hpp"

extern const char test_map_bin;

constexpr u8 ROOM_LIMIT = 48;

static void write_room_to_chrram(u8 id, const Room* room) {
    // Just align them to 256 bytes for now
    vram_adr(id * 0x100);

    // TODO turn off DPCM if its playing
    const u8* cast = reinterpret_cast<const u8*>(room);
    vram_write(cast, sizeof(Room));
}

static void update_exits(u8 id, u8 position, ExitType exit_type) {
    
    const auto offset = id * 0x100 + offsetof(Room, exit);
    vram_adr(offset);
    std::array<u8, 6> exits;
    vram_read(exits.begin(), 6);

    // update the exit
    vram_adr(offset);
    vram_write(exits.begin(), 6);
}

static void roll_exits(Room& room) {
}

static void generate_random_walk() {
    // List of visit tiles
    std::array<u8, 10 * 10> map{};

    // Setup the initial bank
    set_chr_bank(4);

    // Manually build out the root room.
    {
        Room curr;
        curr.room_type = RoomType::Root;
        
        u8 exits = rand8() & 0b00111111;
        for (u8 i=0; i < 6; ++i) {
            if ((exits & i) != 0) {
                // TODO: roll a random exit metatile too?
                curr.exit[i] = static_cast<ExitType>(rand8() & 1);
            }
        }
        write_room_to_chrram(0, &curr);
    }

    for (u8 i=1; i < ROOM_LIMIT; ++i) {
        Room curr;
    }
}

namespace MapLoader {

    void update() {}

    void init() {
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
