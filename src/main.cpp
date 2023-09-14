

#include <neslib.h>
#include <peekpoke.h>
#include <bank.h>

#include "common.hpp"
#include "dungeon_generator.hpp"
#include "game.hpp"
#include "object.hpp"
#include "map_loader.hpp"
#include "title_screen.hpp"

GameMode game_mode;
GameMode prev_game_mode;

__attribute__((section(".zp"))) u8 global_timer[3];

static void main_init() {    
    prev_game_mode = (GameMode) 0xff;
    
    // Disable the PPU so we can freely modify its state
    ppu_off();

    set_mirroring(MIRROR_VERTICAL);

    // set 8x16 sprite mode
    oam_size(1);

    // Use lower half of PPU memory for background tiles
    bank_bg(0);

    // Clear out all the objects
    for (auto obj : objects) {
        obj.state = State::Dead;
    }

    // Initialize the player object
    auto player = objects[0];
    player.metasprite = 0;
    player.animation_frame = 0;
    player.state = State::Hidden;
}


int main() {
  main_init();

  while (true) {
    ppu_wait_nmi();

    pad_poll(0);

    // If we changed game modes, initialze the new one
    if (game_mode != prev_game_mode) {
      switch (game_mode) {
        case GameMode::TitleScreen:
          Titlescreen::init();
          break;
        case GameMode::GenerateDungeon:
          generate_dungeon();
          game_mode = GameMode::MapLoader;
          // FALLTHROUGH
        case GameMode::MapLoader:
          MapLoader::init();
          game_mode = GameMode::GamePlay;
          // FALLTHROUGH
        case GameMode::GamePlay:
            // Game::init();
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
      case GameMode::GamePlay:
        Game::update();
        break;
      default:
        break;
    }
  }
}