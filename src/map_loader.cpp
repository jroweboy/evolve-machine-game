
#include <bank.h>
#include <neslib.h>
#include <peekpoke.h>

#include "common.hpp"
#include "dungeon_generator.hpp"
#include "map_loader.hpp"
#include "map.hpp"
#include "rand.hpp"

extern const char room_chr[];
extern const char kitty_chr[];

// List of compressed room base nametable/attr data
extern const char bottom_bin[];
extern const char left_bin[];
extern const char right_bin[];
extern const char single_bin[];
extern const char start_bin[];
extern const char top_bin[];

constexpr const char* sections[6] = {
    bottom_bin,
    left_bin,
    right_bin,
    single_bin,
    start_bin,
    top_bin,
};

Room room;
Section lead;
Section side;

u8 section_id;

static void read_map_room(u8 room_id) {
    // switch the bank to the bank with the save data
    set_chr_bank(3);


    Dungeon::load_room_from_chrram(room_id);
    Dungeon::load_section_to_lead(room.lead_id);
    Dungeon::load_section_to_side(room.side_id);
}

namespace MapLoader {

    void load_map(u8 section_id) {
        // TODO turn off DPCM if its playing
        // ppu_off();

        // switch to the 16kb bank that holds the level
        set_prg_bank(1);

        // TODO save the previous room state when leaving?
        read_map_room(section_id);

        set_chr_bank(0);
        vram_adr(0x00);
        donut_decompress(&room_chr);

        // always add the kitty tile to the CHR
        vram_adr(0x1000);
        donut_decompress(&kitty_chr);

        vram_adr(((u16)lead.nametable) << 8);
        donut_decompress(sections[static_cast<u8>(lead.room_base)]);

        if (side.room_id != Dungeon::NO_EXIT) {
            vram_adr(((u16)side.nametable) << 8);
            donut_decompress(sections[static_cast<u8>(side.room_base)]);
        }

        pal_bright(0);

        scroll(0, 0);

        set_prg_bank(2);
    }
}
