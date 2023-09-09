
#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#include "chr.hpp"
#include "music.hpp"
#include "rand.hpp"
#include "title_screen.hpp"

extern const char title_screen_bin;

constexpr char background_pal[] = {
    0x0f, 0x16, 0x27, 0x2d,
    0x0f, 0x1c, 0x31, 0x30,
    0x0f, 0x10, 0x20, 0x30,
    0x0f, 0x10, 0x20, 0x30,
};

namespace Titlescreen {
    void init() {
        // copy all chr for the title screen into CHR RAM bank 0
        ppu_off();

        // switch to the 16kb bank that holds the title screen CHR
        set_prg_bank(1);

        // set the address to the start of CHR RAM and decompress the CHR
        set_chr_bank(0);
        vram_adr(0x00);
        donut_decompress(&title_screen_bin);

        vram_adr(NAMETABLE_A);
        vram_unrle(CHR::titlescreen_nametable);

        pal_bg(background_pal);
        pal_bright(0);

        ppu_on_all();

        pal_fade_to(0, 4);

        play_song(Song::Intro);
    }

    void update() {
        u8 pressed = get_pad_new(0);
        if (pressed & PAD_START) {
            Rng::seed(nullptr);
            pal_fade_to(4, 0);
            play_song(Song::StopMusic);
            game_mode = GameMode::GenerateDungeon;
            return;
        }
        // TODO: add a set seed option?
    }
}