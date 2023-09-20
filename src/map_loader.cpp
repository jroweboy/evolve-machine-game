
#include <bank.h>
#include <neslib.h>
#include <peekpoke.h>

#include "common.hpp"
#include "map_loader.hpp"
#include "map.hpp"
#include "rand.hpp"

extern const char room_chr[];
extern const char kitty_chr[];
extern const char start_bin[];

Room room;
Section lead;
Section side;

u8 section_id;

namespace MapLoader {

    void load_map() {
        // TODO turn off DPCM if its playing
        // ppu_off();

        // switch to the 16kb bank that holds the level
        set_prg_bank(1);
        
        set_chr_bank(0);
        vram_adr(0x00);
        donut_decompress(&room_chr);

        vram_adr(NAMETABLE_A);
        donut_decompress(&start_bin);
        // vram_adr(NAMETABLE_B);
        // vram_unrle(CHR::test_bg_nametable2);

        vram_adr(0x1000);
        donut_decompress(&kitty_chr);

        // pal_bg(background_pal);
        pal_bright(0);

        scroll(0, 0);
    }
}
