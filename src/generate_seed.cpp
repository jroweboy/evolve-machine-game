
#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "chr.hpp"
#include "music.hpp"
#include "generate_seed.hpp"

extern const char test_map_bin;

namespace GenerateSeed {
    void init() {
        constexpr char background_pal[] = {
            0x0f, 0x16, 0x27, 0x2d,
            0x0f, 0x1c, 0x31, 0x30,
            0x0f, 0x10, 0x20, 0x30,
            0x0f, 0x10, 0x20, 0x30,
        };

        ppu_off();

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

        play_song(Music::TITLE_AMBIENCE);
    }

    void update() {
    }
}
