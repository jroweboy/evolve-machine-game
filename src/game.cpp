
#include <fixed_point.h>
#include <neslib.h>
#include <mapper.h>
#include <peekpoke.h>

#include "common.hpp"
#include "game.hpp"
#include "dungeon_generator.hpp"
#include "map.hpp"
#include "music.hpp"
// #include "graphics.hpp"
// #include "map_loader.hpp"
#include "map_loader.hpp"
#include "nes_extra.hpp"
#include "object.hpp"
#include "sprite_render.hpp"


constexpr auto PLAYER_MOVESPEED = 1.20_8_8;


extern volatile char FRAME_CNT1;

// marks if the previous frame was a lag frame.
noinit static bool lag_frame;

noinit static GameMode game_mode;
noinit static GameMode prev_game_mode;

// When loading the room, save the room bounds here to cache them for offscreen checking.
noinit static s16 room_bounds_x;
noinit static s16 room_bounds_y;

prg_rom_2 extern "C" u8 check_solid_collision(u8 filter, u8 obj_idx);

struct ObjectCollisionParameter {
    s16 x;
    s16 y;
    s8 width;
    s8 height;
};
zpnoinit extern ObjectCollisionParameter obj_collision_parameter;
prg_rom_2 extern "C" u8 check_object_collision(u8 filter, u8 obj_idx);

prg_rom_2 static void load_collision_parameter(u8 obj_idx) {
    const auto obj = objects[obj_idx];
    obj_collision_parameter.x = obj.x->as_i() + obj.hitbox.x;
    obj_collision_parameter.y = obj.y->as_i() + obj.hitbox.y;
    obj_collision_parameter.width = obj.hitbox.width;
    obj_collision_parameter.height = obj.hitbox.height;
}

prg_rom_2 static void check_player_collision() {
    load_collision_parameter(0);
    [[maybe_unused]] auto player = objects[0];
    for (u8 i = 2; i < OBJECT_COUNT; ++i) {
        auto obj = objects[i];
        u8 collision = check_object_collision((u8)CollisionType::All, i);
        if (collision == 0)
             continue;
        const auto pad_new = get_pad_new(0);
        if ((collision & CollisionType::Pickup) != 0) {
            if (obj.state == State::GroundedWeapon && (pad_new & PAD_A) != 0) {
                auto weapon = objects[1];
                Metasprite tmp = weapon.metasprite.get();
                weapon.metasprite = obj.metasprite.get();
                weapon.state = State::EquippedWeapon;
                weapon.attribute = obj.attribute.get();
                weapon.tile_offset = obj.tile_offset.get();
                if (tmp > Metasprite::None) {
                    obj.metasprite = tmp;
                    obj.state = State::GroundedWeapon;
                } else {
                    obj.state = State::Hidden;
                }
            }
        }
    }
}

prg_rom_2 static void move_player() {
    auto player = objects[0];
    auto pressed = pad_state(0);
    s16 original_y = player.y->as_i();
    if (pressed & PAD_UP) {
        player.direction = Direction::Up;
        player.metasprite = Metasprite::KittyUp;
        player.y = player.y.get() - PLAYER_MOVESPEED;
    } else if (pressed & PAD_DOWN) {
        player.direction = Direction::Down;
        player.metasprite = Metasprite::KittyDown;
        player.y = player.y.get() + PLAYER_MOVESPEED;
    }
    if (player.y->as_i() != original_y) {
        u8 collision = check_solid_collision(CollisionType::All, 0);
        if (collision > 0) {
            player.y = original_y;
        }
    }
    s16 original_x = player.x->as_i();
    if (pressed & PAD_LEFT) {
        player.direction = Direction::Left;
        player.metasprite = Metasprite::KittyLeft;
        player.x = player.x.get() - PLAYER_MOVESPEED;
    } else if (pressed & PAD_RIGHT) {
        player.direction = Direction::Right;
        player.metasprite = Metasprite::KittyRight;
        player.x = player.x.get() + PLAYER_MOVESPEED;
    }
    if (player.x->as_i() != original_x) {
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

constexpr s8 WEAPON_BOB_Y_ORIGIN = -10;
static const s8 weapon_bob_y_offset[] = {
    WEAPON_BOB_Y_ORIGIN - 0,
    WEAPON_BOB_Y_ORIGIN - 0,
    WEAPON_BOB_Y_ORIGIN - 1,
    WEAPON_BOB_Y_ORIGIN - 2,
    WEAPON_BOB_Y_ORIGIN - 3,
    WEAPON_BOB_Y_ORIGIN - 3,
    WEAPON_BOB_Y_ORIGIN - 2,
    WEAPON_BOB_Y_ORIGIN - 1
};

prg_rom_2 static void run_weapon_bob() {
    auto weapon = objects[1];
    
    if ((weapon.state & State::Hidden) != 0) {
        // No weapon equipped so dont draw it
        return;
    }
    auto player = objects[0];
    
    // reuse the weapon HP and iframe as the bob timer
    weapon.x = player.x.get() - 4;
    weapon.y = player.y.get() + weapon_bob_y_offset[weapon.hp];

    weapon.iframe = (weapon.iframe + 1) & 0b00000111;
    if (weapon.iframe == 0) {
        weapon.hp = (weapon.hp + 1) & 0b00000111;
    }

    // if the player uses an attack, animate the spin
    if ((get_pad_new(0) & PAD_B) != 0 && weapon.frame_counter < 0) {
        weapon.frame_counter = 32;
        sfx_queue1 = Sfx::weapon_fire_1;
    }
    if (weapon.frame_counter >= 0) {
        weapon.frame_counter--;
        if (weapon.frame_counter == 24 || weapon.frame_counter == 16 || weapon.frame_counter == 8 || weapon.frame_counter == 0) {
            weapon.animation_frame = (weapon.animation_frame + 1) & 0b11;
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
        s16 screen_pos_y = player.y->as_i() - room.y - view_y;
        if (screen_pos_y < 0 || screen_pos_y > 239) {
            return;
        }
        // if (screen_pos_y > 0x88 && view_y < 240) {
        //     view_y += PLAYER_MOVESPEED;
        // } else if (screen_pos_y < 0x68 && view_y != 0) {
        //     view_y -= PLAYER_MOVESPEED;
        // }
        u8 orig_view_y = view_y;
        if (screen_pos_y > 0x78) {
            if (view_y < 240) {
                if (screen_pos_y >= 0x80) {
                    view_y += 2;
                } else {
                    view_y += 1;
                }
                if (orig_view_y > view_y) {
                    view_y = 240;
                }
            }
        } else if (screen_pos_y < 0x58)  {
            if (view_y > 0) {
                if (screen_pos_y < 0x50) {
                    view_y -= 2;
                } else {
                    view_y -= 1;
                }
                if (orig_view_y < view_y) {
                    view_y = 0;
                }
            }
        }
    } else {
        s16 screen_pos_x = player.x->as_i() - room.x - view_x;
        if (screen_pos_x < 0 || screen_pos_x > 255) {
            return;
        }
        u8 orig_view_x = view_x;
        if (screen_pos_x > 0x88) {
            if (view_x < 255) {
                if (screen_pos_x >= 0x90) {
                    view_x += 2;
                } else {
                    view_x += 1;
                }
                if (orig_view_x > view_x) {
                    view_x = 255;
                }
            }
        } else if (screen_pos_x < 0x78) {
            if (view_x > 0) {
                if (screen_pos_x < 0x70) {
                    view_x -= 2;
                } else {
                    view_x -= 1;
                }
                if (orig_view_x < view_x) {
                    view_x = 0;
                }
            }
        }
    }
    scroll(view_x, view_y);
}

prg_rom_2 static void check_screen_transition() {
    auto player = objects[0];
    // since this check happens every frame, we cache the room boundaries
    if (player.x->as_i() < room.x || player.x->as_i() > room_bounds_x 
        || player.y->as_i() < room.y || player.y->as_i() > room_bounds_y) {
        game_mode = GameMode::MapLoader;
    }
}

prg_rom_2 static Direction get_direction() {
    auto player = objects[0];
    if (player.x->as_i() < room.x) {
        return Direction::Left;
    } else if (player.y->as_i() < room.y) {
        return Direction::Up;
    } else if (player.x->as_i() > room_bounds_x) {
        return Direction::Right;
    } else if (player.y->as_i() > room_bounds_y) {
        return Direction::Down;
    }
    return Direction::Error;
}

prg_rom_2 static void calculate_screen_bounds() {
    switch (room.scroll) {
    case ScrollType::Single:
        room_bounds_x = room.x + 256 - 16;
        room_bounds_y = room.y + 240 - 16;
        break;
    case ScrollType::Horizontal:
        room_bounds_x = room.x + 256 * 2 - 16;
        room_bounds_y = room.y + 240 - 16;
        break;
    case ScrollType::Vertical:
        room_bounds_x = room.x + 256 - 16;
        room_bounds_y = room.y + 240 * 2 - 16;
        break;
    }
}

// __attribute__((section(".prg_rom_2.x_offset_lut"))) static const s8 player_position_x_offset[] = {
//     // Scroll type Single
//     0, 16, 0, 224,
//     // Scroll type Horizontal
//     0, 16, 0, 256 + 224,
//     // Scroll type Vertical
//     0, 16, 0, 224,
// };
// __attribute__((section(".prg_rom_2.y_offset_lut"))) static const s8 player_position_y_offset[] = {
//     // Scroll type Single
//     224, 0, 16, 0,
//     // Scroll type Horizontal
//     224, 0, 16, 0,
//     // Scroll type Vertical
//     240 + 226, 0, 16, 0,
// };

// prg_rom_2 static void correct_player_position(Direction dir, s16 room_x, s16 room_y) {
//     // after a map transition we might need to shift the player a bit to match up with the new map exit
//     auto player = objects[0];

//     // Recalculate the player position so that they are on the place on the screen, but on the opposite direction in the new room
//     switch (dir) {
//     case Direction::Down:
//         screen_y = (240 - screen_y) + 8;
//         player.y = f16_8();
//         break;
//     case Direction::Up:
//         screen_y = (232 + screen_y) - 8;
//         break;
//     case Direction::Right:
//         screen_x = (255 - screen_x) + 8;
//         break;
//     case Direction::Left:
//         screen_x = (255 + screen_x) + 8;
//         break;
//     case Direction::Error:
//         break;
//     }



//     // view_x = 0;
//     // view_y = 0;

//     // don't add any offset if its the intro cutscene.
//     // if (direction != Direction::Error) {
//         // u8 offset = (((u8)room.scroll << 2) & 0b00001100) | (u8) direction;
//         // s16 x_offset = player_position_x_offset[offset];
//         // s16 y_offset = player_position_y_offset[offset];

//         // if (x_offset) {
//         //     player.x = f16_8(room.x + x_offset);
//         // } else if (y_offset) {
//         //     player.y = f16_8(room.y + y_offset);
//         // }
//     // }
// }

prg_rom_2 static void update_scroll() {
    auto player = objects[0];
    view_x = 0;
    view_y = 0;
    switch (room.scroll) {
    case ScrollType::Horizontal:
        // if (((player.x->as_i() >> 8) - (room.x >> 8)) > 0) {
        //     view_x = 0xff;
        // }
        view_x = MMAX(player.x->as_i() - room.x - 0x78, 0);
        break;
    case ScrollType::Vertical:
        // if (((player.y->as_i() >> 8) - (room.y >> 8)) > 0) {
        //     view_y = 0xf0;
        // }
        view_y = MMAX(player.y->as_i() - room.y - 0x68, 0);
        break;
    default:
        break;
    }
    scroll(view_x, view_y);
}


prg_rom_2 static void load_new_map() {
    // find what room we exited to. there's probably a smarter way to do this, but its during
    // a loading transition so i don't care too much to optimize it.
    // save previous room 
    set_chr_bank(3);

    Dungeon::write_room_to_chrram(lead.room_id);
    Dungeon::write_section_lead(room.lead_id);
    Dungeon::write_section_side(room.side_id);

    u8 section_id = room.lead_id;
    auto player = objects[0];

    Direction direction = get_direction();
    SectionBase vertical_section;
    bool is_bottom_section = player.y->as_i() - room.y > 240;
    if (lead.room_base == SectionBase::StartDown || lead.room_base == SectionBase::StartUp) {
        vertical_section = is_bottom_section ? SectionBase::StartDown : SectionBase::StartUp;
    } else {
        vertical_section = is_bottom_section ? SectionBase::Bottom : SectionBase::Top;
    }
    
    SectionBase horizontal_section = player.x->as_i() - room.x > 255 ? SectionBase::Right : SectionBase::Left;
    u8 direction_a = Dungeon::GetNeighborId(room.lead_id, (u8)direction);
    u8 direction_b = Dungeon::GetNeighborId(room.lead_id, (u8)direction);
    switch (room.scroll) {
    case ScrollType::Single:
        section_id = direction_a;
        break;
    case ScrollType::Horizontal:
        section_id = lead.room_base == horizontal_section
            ? direction_a
            : direction_b;
        break;
    case ScrollType::Vertical:
        section_id = lead.room_base == vertical_section
            ? direction_a
            : direction_b;
        break;
    }

    MapLoader::load_map(section_id);
    // build the new map bounds
    calculate_screen_bounds();

    // And now correct the position of the character
    auto& new_section = room.lead_id == section_id ? lead : side;
    s16 new_x = player.x->as_i();
    s16 new_y = player.y->as_i();
    if (vertical_section == SectionBase::Bottom || vertical_section == SectionBase::StartDown) {
        new_y += 16;
    }
    switch (direction) {
    case Direction::Up:
        new_y -= 32;
        break;
    case Direction::Right:
        new_x += 16;
        break;
    case Direction::Down:
        new_y += 32;
        break;
    case Direction::Left:
        new_x -= 16;
        break;
    case Direction::Error:
        break;
    }
    switch (new_section.room_base) {
    case SectionBase::Bottom:
    case SectionBase::StartDown:
        new_y -= 16;
    case SectionBase::Left:
    case SectionBase::Right:
    case SectionBase::Single:
    case SectionBase::StartUp:
    case SectionBase::Top:
    case SectionBase::Count:
      break;
    }
    
    player.x = f16_8(new_x);
    player.y = f16_8(new_y);

    // correct_player_position(direction, );
    update_scroll();

    game_mode = GameMode::InGame;
}

constexpr char sprites_pal[] = {
    0x0f, 0x03, 0x00, 0x27,
    0x0f, 0x06, 0x26, 0x30,
    0x0f, 0x10, 0x20, 0x30,
    0x0f, 0x10, 0x20, 0x30,
};

namespace Game {
prg_rom_2 void init() {
    // ppu_wait_nmi();
    // ppu_off();
    pal_spr(&sprites_pal);
    prev_game_mode = GameMode::MapLoader;
    game_mode = GameMode::InGame;
    calculate_screen_bounds();
    update_scroll();
}

prg_rom_2 void update() {
    if (game_mode != prev_game_mode) {
        switch (game_mode) {
        case GameMode::MapLoader:
            pal_fade_to(4, 0, 2);
            ppu_off();
            load_new_map();
            // fallthrough
        case GameMode::InGame:
            ppu_on_all();
            pal_fade_to(0, 4, 2);
            break;
        case GameMode::Pause:
            return;
        }
        prev_game_mode = game_mode;
    }
    u8 frame = FRAME_CNT1;

    POKE(0x4123, 3);
    move_player();
    check_player_collision();
    run_weapon_bob();
    scroll_screen();
    check_screen_transition();
    POKE(0x4123, 4);

    // Skip drawing sprites this frame if we lagged on the previous frame.
    // This should help us get caught up if we process too much in one frame
    if (!lag_frame) {
        // Sprite::move_sprites_offscreen();
        draw_all_metasprites();
    }
    lag_frame = frame != FRAME_CNT1;
    
    POKE(0x4123, 0);
}
}