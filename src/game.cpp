
#include <neslib.h>
#include <bank.h>

#include "common.hpp"
#include "game.hpp"
#include "map_loader.hpp"
#include "nes_extra.hpp"
#include "object.hpp"
#include "sprite_render.hpp"

extern volatile char FRAME_CNT1;

// marks if the previous frame was a lag frame.
static bool lag_frame;

static GameMode game_mode;
static GameMode prev_game_mode;

static void move_player() {
    auto player = objects[0];
    auto pressed = pad_state(0);
    if (pressed & PAD_UP) {
        player.direction = Direction::Up;
        player.metasprite = 1;
        player.y--;
    } else if (pressed & PAD_DOWN) {
        player.direction = Direction::Down;
        player.metasprite = 2;
        player.y++;
    }
    if (pressed & PAD_LEFT) {
        player.direction = Direction::Left;
        player.metasprite = 3;
        player.x--;
    } else if (pressed & PAD_RIGHT) {
        player.direction = Direction::Right;
        player.metasprite = 0;
        player.x++;
    }
    if (pressed & (PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_UP)) {
        player.frame_counter--;
        if (player.frame_counter < 0) {
            player.frame_counter = 6;
            player.animation_frame = (player.animation_frame + 1) & 0b11;
        }
    }
}

constexpr char sprites_pal[] = {
    0x0f, 0x03, 0x00, 0x27,
    0x0f, 0x1c, 0x31, 0x30,
    0x0f, 0x10, 0x20, 0x30,
    0x0f, 0x10, 0x20, 0x30,
};

namespace Game {
void init() {
    ppu_wait_nmi();
    ppu_off();
    pal_spr(&sprites_pal);
    prev_game_mode = GameMode::MapLoader;
    game_mode = GameMode::InGame;
    set_prg_bank(2);
    auto player = objects[0];
    player.metasprite = 0;
    player.state = (State)0;
    player.x = 100;
    player.y = 100;
}

void update() {
    if (game_mode != prev_game_mode) {
        if (game_mode == GameMode::MapLoader) {
            pal_fade_to(4, 2, 2);
            ppu_off();
        } else if (game_mode == GameMode::InGame) {
            ppu_on_all();
            pal_fade_to(0, 4, 2);
        } else if (game_mode == GameMode::Pause) {
            return;
        }
        prev_game_mode = game_mode;
    }
    u8 frame = FRAME_CNT1;

    move_player();

    // Skip drawing sprites this frame if we lagged on the previous frame.
    // This should help us get caught up if we process too much in one frame
    if (!lag_frame) {
        Sprite::move_sprites_offscreen();
        Sprite::draw_objects();
    }
    lag_frame = frame != FRAME_CNT1;
}
}