#include <cstdio>

#include <nesdoug.h>
#include <neslib.h>
#include <peekpoke.h>
#include <bank.h>

#include "music.hpp"
#include "title_screen.hpp"

// constexpr char kScreenWidth = 32;
// constexpr char kScreenHeight = 30;
// constexpr int kScreenSize = kScreenWidth * kScreenHeight;

// constexpr char hello[] = "Hello, NES!";

// void donut_decompress_block(const u16* data) {
//   POKEW(0xed, *data);
//   POKE(0xef, 0);
//   ((void(*)(void))0xc000)();
// }

// constexpr char background_pal[] = {
//     0x0f, 0x10, 0x20, 0x30, // grayscale
//     0x0f, 0x10, 0x20, 0x30, // grayscale
//     0x0f, 0x10, 0x20, 0x30, // grayscale
//     0x0f, 0x10, 0x20, 0x30, // grayscale
// };
GameMode game_mode;
GameMode prev_game_mode;

int main() {
  prev_game_mode = (GameMode) 0xff;
  // Disable the PPU so we can freely modify its state
  ppu_off();

  // Use lower half of PPU memory for background tiles
  bank_bg(0);

  while (true) {
    __attribute__((unused)) const char pad_state = pad_poll(0);

    // If we changed game modes, initialze the new one
    if (game_mode != prev_game_mode) {
      switch (game_mode) {
        case GameMode::TitleScreen:
          Titlescreen::init();
          break;
        default:
          break;
      }
      prev_game_mode = game_mode;
    }
    
    // Run this frame of the game mode
    switch (game_mode) {
      case GameMode::TitleScreen:
        Titlescreen::update();
        break;
      default:
        break;
    }
  }

  // // Set the background palette
  // pal_bg(background_pal);

  // // Fill the background with space characters to clear the screen
  // vram_adr(NAMETABLE_A);
  // vram_fill(' ', kScreenSize);

  // // Write a message
  // vram_adr(NTADR_A(10, 10));
  // vram_write(hello, sizeof(hello) - 1);

  // Turn the PPU back on
  // ppu_on_all();

  // char current_mirroring = MIRROR_HORIZONTAL;
  // set_mirroring(current_mirroring);

  // char palette_color = 0;
  // char counter = 0;
  // for (;;) {
  //   // Wait for the NMI routine to end so we can start working on the next frame
  //   ppu_wait_nmi();

  //   // Note: if you don't poll a controller during a frame, emulators will
  //   // report that as lag
  //   __attribute__((unused)) const char pad_state = pad_poll(0);

  //   if(counter == 0) {
  //     if (++palette_color == 64) palette_color = 0;

  //     current_mirroring = current_mirroring == MIRROR_HORIZONTAL ? MIRROR_VERTICAL : MIRROR_HORIZONTAL;
  //     set_mirroring(current_mirroring);
  //     // Tell `neslib` that we want to do a buffered background data transfer
  //     // this frame
  //     // set_vram_buffer();
  //     // char buffer[4];
  //     // std::snprintf(buffer, sizeof(buffer), "$%02x", static_cast<int>(palette_color));
  //     // multi_vram_buffer_horz(buffer, 3, NTADR_A(14, 12));
  //   }

  //   // TODO: interesting stuff
  //   pal_col(0, palette_color);

  //   if (++counter == 30) counter = 0;
  // }
}