
#include <mapper.h>
#include <neslib.h>

#include "graphics.hpp"
#include "header/graphics_constants.hpp"
#include "nes_extra.hpp"
#include "common.hpp"
#include "music.hpp"
#include "rand.hpp"
#include "title_screen.hpp"

// constexpr char background_pal[] = {
//     0x0f, 0x16, 0x27, 0x2d,
//     0x0f, 0x1c, 0x31, 0x30,
//     0x0f, 0x10, 0x20, 0x30,
//     0x0f, 0x10, 0x20, 0x30,
// };

namespace Titlescreen {

    enum class State {
        Waiting,
        EnterSeed,
    };

    State state;

    void init() {
        // copy all chr for the title screen into CHR RAM bank 0
        ppu_off();

        // switch to the 16kb bank that holds the title screen CHR
        set_prg_bank(1);

        // set the address to the start of CHR RAM and decompress the CHR
        set_chr_bank(0);
        vram_adr(0x00);
        // donut_decompress(&title_chr);
        donut_decompress_vram(Archive::titlescreen_chr);

        vram_adr(NAMETABLE_A);
        donut_decompress_vram(Archive::titlescreen_atr_nmt);
        // donut_decompress(&title_bin);

        donut_decompress_buffer(Archive::titlescreen_pal);
        pal_bg(decompress_buffer);
        pal_bright(0);


        if (!has_epsm) {
            // EPSM is not available so cover up the EPSM logo in the corner
            vram_adr(0x2336);
            vram_fill(0x00, 6);
            vram_adr(0x2356);
            vram_fill(0x00, 6);
            vram_adr(0x2376);
            vram_fill(0x00, 6);
        }


        next_song = Song::Intro;

        ppu_on_all();

        pal_fade_to(0, 4, 8);

        state = State::Waiting;
        
        // Set the initial RNG seed to the frame counter here. This
        // isn't good enough to get a real RNG seed.
        Rng::seed(nullptr);
    }

    prg_rom_2 void update() {
        // TODO: make the stars palette cycle
        if (state == State::Waiting) {
            // Burn through some RNG numbers while waiting.
            for (u8 i=0; i < 35; ++i) {
                u8 pressed = pad_poll(0);
                u8* raw_seed = reinterpret_cast<u8*>(&::seed);
                raw_seed[i & 0b11]++;
                // Run the RNG once to push the numbers along
                Rng::rand8();
                if (pressed & PAD_START) {
                    next_song = Song::StopMusic;
                    pal_fade_to(4, 0, 4);
                    main_mode = MainMode::GenerateDungeon;
                    return;
                } else if (pressed & PAD_SELECT) {
                    state = State::EnterSeed;
                }
            }
        } else if (state == State::EnterSeed) {
            // TODO seed input menu.
            state = State::Waiting;
        }
    }
}