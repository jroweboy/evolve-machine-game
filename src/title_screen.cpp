
#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "chr.hpp"
#include "title_screen.hpp"

namespace Titlescreen {
    void init() {
        // copy all chr for the title screen into CHR RAM bank 0
        ppu_off();
        set_chr_bank(0);

        // copy data from 
        vram_adr(0x00);

        vram_adr(NAMETABLE_A);
        vram_unrle(CHR::titlescreen_nametable);
        ppu_on_all();

        pal_fade_to(0, 4);
    }

    void update() {

    }
}