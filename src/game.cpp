
#include <neslib.h>
#include <mapper.h>
#include <peekpoke.h>

#include "common.hpp"
#include "game.hpp"
#include "map.hpp"
// #include "graphics.hpp"
// #include "map_loader.hpp"
#include "nes_extra.hpp"
#include "object.hpp"
#include "sprite_render.hpp"

extern volatile char FRAME_CNT1;

// marks if the previous frame was a lag frame.
static bool lag_frame;

static GameMode game_mode;
static GameMode prev_game_mode;

prg_rom_2 extern "C" u8 check_solid_collision(u8 filter, u8 obj_idx);

prg_rom_2 static void move_player() {
    auto player = objects[0];
    auto pressed = pad_state(0);
    s16 original_y = player.y;
    if (pressed & PAD_UP) {
        player.direction = Direction::Up;
        player.metasprite = Metasprite::KittyUp;
        player.y--;
    } else if (pressed & PAD_DOWN) {
        player.direction = Direction::Down;
        player.metasprite = Metasprite::KittyDown;
        player.y++;
    }
    if (player.y != original_y) {
        u8 collision = check_solid_collision(CollisionType::All, 0);
        if (collision > 0) {
            player.y = original_y;
        }
    }
    s16 original_x = player.x;
    if (pressed & PAD_LEFT) {
        player.direction = Direction::Left;
        player.metasprite = Metasprite::KittyLeft;
        player.x--;
    } else if (pressed & PAD_RIGHT) {
        player.direction = Direction::Right;
        player.metasprite = Metasprite::KittyRight;
        player.x++;
    }
    if (player.x != original_x) {
        u8 collision = check_solid_collision(CollisionType::All, 0);
        if (collision > 0) {
            player.x = original_x;
        }
    }
    if (pressed & (PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_UP)) {
        player.frame_counter--;
        if (player.frame_counter < 0) {
            player.frame_counter = 6;
            player.animation_frame = (player.animation_frame + 1) & 0b11;
        }
    }
}

prg_rom_2 static void scroll_screen() {
    if (room.scroll == ScrollType::Single) {
        return;
    }
    // This room has scroll in it
    auto player = objects[0];
    if (room.scroll == ScrollType::Vertical) {
        s16 screen_pos_y = player.y - room.y - view_y;
        if (screen_pos_y < 0 || screen_pos_y > 239) {
            return;
        }
        if (screen_pos_y > 0x88 && view_y != 240) {
            view_y += 1;
        }
        if (screen_pos_y < 0x68 && view_y != 0) {
            view_y -= 1;
        }
    } else {
        s16 screen_pos_x = player.x - room.x - view_x;
        if (screen_pos_x < 0 || screen_pos_x > 255) {
            return;
        }
        if (screen_pos_x > 0x90 && view_x != 255) {
            view_x += 1;
        }
        if (screen_pos_x < 0x70 && view_x != 0) {
            view_x -= 1;
        }
    }
    scroll(view_x, view_y);
}

constexpr char sprites_pal[] = {
    0x0f, 0x03, 0x00, 0x27,
    0x0f, 0x1c, 0x31, 0x30,
    0x0f, 0x10, 0x20, 0x30,
    0x0f, 0x10, 0x20, 0x30,
};

namespace Game {
prg_rom_2 void init() {
    ppu_wait_nmi();
    ppu_off();
    pal_spr(&sprites_pal);
    prev_game_mode = GameMode::MapLoader;
    game_mode = GameMode::InGame;
    set_prg_bank(2);
}

prg_rom_2 void update() {
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

    POKE(0x4123, 3);
    move_player();
    scroll_screen();
    POKE(0x4123, 4);

    // Skip drawing sprites this frame if we lagged on the previous frame.
    // This should help us get caught up if we process too much in one frame
    if (!lag_frame) {
        Sprite::move_sprites_offscreen();
        Sprite::draw_objects();
    }
    lag_frame = frame != FRAME_CNT1;
    
    POKE(0x4123, 0);
}
}