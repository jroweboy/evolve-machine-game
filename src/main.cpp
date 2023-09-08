

#include <nesdoug.h>
#include <neslib.h>
#include <peekpoke.h>
#include <bank.h>

#include "common.hpp"
#include "map_loader.hpp"
#include "title_screen.hpp"

GameMode game_mode;
GameMode prev_game_mode;

u8 global_timer[3];

int main() {
  prev_game_mode = (GameMode) 0xff;
  
  // Disable the PPU so we can freely modify its state
  ppu_off();

  set_mirroring(MIRROR_VERTICAL);

  // set 8x16 sprite mode
  oam_size(1);

  // Use lower half of PPU memory for background tiles
  bank_bg(0);

  while (true) {
    ppu_wait_nmi();

    pad_poll(0);
    // If we changed game modes, initialze the new one
    if (game_mode != prev_game_mode) {
      switch (game_mode) {
        case GameMode::TitleScreen:
          Titlescreen::init();
          break;
        case GameMode::MapLoader:
          MapLoader::init();
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
      case GameMode::MapLoader:
        MapLoader::update();
        break;
      default:
        break;
    }
  }
}