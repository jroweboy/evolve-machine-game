
#include <neslib.h>
#include <mapper.h>
#include <peekpoke.h>

#include "common.hpp"
#include "game.hpp"
#include "dungeon_generator.hpp"
#include "map.hpp"
// #include "graphics.hpp"
// #include "map_loader.hpp"
#include "map_loader.hpp"
#include "nes_extra.hpp"
#include "object.hpp"
#include "sprite_render.hpp"

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
noinit extern ObjectCollisionParameter obj_collision_parameter;
prg_rom_2 extern "C" u8 check_object_collision(u8 filter, u8 obj_idx);

constexpr static void load_collision_parameter(u8 obj_idx) {
    const auto obj = objects[obj_idx];
    obj_collision_parameter.x = obj.x + obj.hitbox.x;
    obj_collision_parameter.y = obj.y + obj.hitbox.y;
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
                if (tmp > (Metasprite)0) {
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

constexpr s8 WEAPON_BOB_Y_ORIGIN = -10;
static constexpr s8 weapon_bob_y_offset[] = {
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
    
    // reuse the weapon HP as the bob timer
    weapon.x = player.x.get() - 4;
    weapon.y = player.y.get() + weapon_bob_y_offset[weapon.hp];

    weapon.frame_counter = (weapon.frame_counter + 1) & 0b00000111;
    if (weapon.frame_counter == 0) {
        weapon.hp = (weapon.hp + 1) & 0b00000111;
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

prg_rom_2 static void check_screen_transition() {
    auto player = objects[0];
    // since this check happens every frame, we cache the room boundaries
    if (player.x < room.x || player.x > room_bounds_x || player.y < room.y || player.y > room_bounds_y) {
        game_mode = GameMode::MapLoader;
    }
}

prg_rom_2 static Direction get_direction() {
    auto player = objects[0];
    if (player.x < room.x) {
        return Direction::Left;
    } else if (player.y < room.y) {
        return Direction::Up;
    } else if (player.x > room_bounds_x) {
        return Direction::Right;
    } else if (player.y > room_bounds_y) {
        return Direction::Down;
    }
    return Direction::Error;
}

prg_rom_2 static void calculate_screen_bounds() {
    switch (room.scroll) {
    case ScrollType::Single:
        room_bounds_x = room.x + 256;
        room_bounds_y = room.y + 240;
        break;
    case ScrollType::Horizontal:
        room_bounds_x = room.x + 256 * 2;
        room_bounds_y = room.y + 240;
        break;
    case ScrollType::Vertical:
        room_bounds_x = room.x + 256;
        room_bounds_y = room.y + 240 * 2;
        break;
    }
}

__attribute__((section(".prg_rom_2.x_offset_lut"))) static const s16 player_position_x_offset[8] = {
    // Scroll type Horizontal
    0, 16, 0, 256 + 224,
    // Scroll type Vertical
    0, 16, 0, 224,
};
__attribute__((section(".prg_rom_2.y_offset_lut"))) static const s16 player_position_y_offset[8] = {
    // Scroll type Horizontal
    224, 0, 16, 0,
    // Scroll type Vertical
    240 + 226, 0, 16, 0,
};

prg_rom_2 static void correct_player_position(Direction direction) {
    // after a map transition we might need to shift the player a bit to match up with the new map exit
    auto player = objects[0];
    view_x = 0;
    view_y = 0;

    // don't add any offset if its the intro cutscene.
    if (direction != Direction::Error && room.scroll != ScrollType::Single) {
        u8 offset = (((u8)room.scroll << 1) & 0b00000100) | (u8) direction;
        s16 x_offset = player_position_x_offset[offset];
        s16 y_offset = player_position_y_offset[offset];
        if (x_offset) {
            player.x = room.x + x_offset;
        } else if (y_offset) {
            player.y = room.y + y_offset;
        }
    }
    
    switch (room.scroll) {
    case ScrollType::Horizontal:
        view_x = MMAX(player.x - room.x - 0x88, 0);
        break;
    case ScrollType::Vertical:
        view_y = MMAX(player.y - room.y - 0x78, 0);
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
    Dungeon::write_room_to_chrram(lead.room_id);
    Dungeon::write_section_lead(room.lead_id);
    Dungeon::write_section_side(room.side_id);

    u8 section_id = room.lead_id;
    auto player = objects[0];

    Direction direction = get_direction();
    SectionBase vertical_section;
    bool is_bottom_section = player.y - room.y > 240;
    if (lead.room_base == SectionBase::StartDown || lead.room_base == SectionBase::StartUp) {
        vertical_section = is_bottom_section ? SectionBase::StartDown : SectionBase::StartUp;
    } else {
        vertical_section = is_bottom_section ? SectionBase::Bottom : SectionBase::Top;
    }
    SectionBase horizontal_section = player.x - room.x > 255 ? SectionBase::Right : SectionBase::Left;
    switch (room.scroll) {
    case ScrollType::Single:
        section_id = lead.exit[(u8)direction];
        break;
    case ScrollType::Horizontal:
        section_id = lead.room_base == horizontal_section
            ? lead.exit[(u8)direction]
            : side.exit[(u8)direction];
        break;
    case ScrollType::Vertical:
        section_id = lead.room_base == vertical_section
            ? lead.exit[(u8)direction]
            : side.exit[(u8)direction];
        break;
    }
    MapLoader::load_map(section_id);
    // build the new map bounds
    calculate_screen_bounds();
    // And now correct the position of the character
    correct_player_position(direction);

    game_mode = GameMode::InGame;
}

constexpr char sprites_pal[] = {
    0x0f, 0x03, 0x00, 0x27,
    0x0f, 0x1c, 0x31, 0x30,
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
    correct_player_position(Direction::Error);
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
        Sprite::move_sprites_offscreen();
        Sprite::draw_objects();
    }
    lag_frame = frame != FRAME_CNT1;
    
    POKE(0x4123, 0);
}
}