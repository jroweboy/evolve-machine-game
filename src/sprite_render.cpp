
#include <mapper.h>
#include <neslib.h>
#include <soa.h>

#include "map.hpp"
#include "sprite_render.hpp"
#include "common.hpp"
#include "object.hpp"

struct OAMData {
    u8 y;
    u8 tile;
    u8 attr;
    u8 x;
};


__attribute__((section(".zp"))) u8 view_x;
__attribute__((section(".zp"))) u8 view_y;
extern const u8 kitty_walk_right[];
extern const u8 kitty_walk_up[];
extern const u8 kitty_walk_down[];
extern const u8 kitty_walk_left[];

extern const u8 weapon_sphere[];
extern const u8 weapon_pyramid[];
extern const u8 weapon_diamond[];
extern const u8 weapon_cube[];

__attribute__((section(".prg_rom_2.metasprite_table")))
static const u8* metasprite_table[(u8)Metasprite::Count] = {
    kitty_walk_right,
    kitty_walk_up,
    kitty_walk_down,
    kitty_walk_left,
    
    weapon_sphere,
    weapon_pyramid,
    weapon_diamond,
    weapon_cube,
};

noinit static u8 shuffle_offset;
noinit static u8 sprite_slot;

extern u8 OAM_BUF[256];
// volatile OAMData* OAM = &*reinterpret_cast<volatile OAMData*>(&OAM_BUF);

prg_rom_2 static void draw_object(u8 id) {
    const auto& object = objects[id];

    // If the high bit of the object state is 1 then its not going to be drawn so skip it
    if ((object.state & State::Hidden) != 0x0) {
        return;
    }

    s16 screen_x = object.x - room.x - view_x;
    s16 screen_y = object.y - room.y - view_y;

    auto frame = metasprite_table[static_cast<u8>(object.metasprite.get())];
    // auto addr = reinterpret_cast<const u16*>(spr)[object.direction];
    // auto frame = reinterpret_cast<u8*>(addr);
    auto offset = frame[object.animation_frame];
    while (frame[offset] != 0x7f) {
        auto& oam = *reinterpret_cast<OAMData*>(&OAM_BUF[sprite_slot]);
        s16 x = ((s8)frame[offset--]) + screen_x;
        if (x < 0 || x > 255) {
            offset -= 3;
            continue;
        }
        s16 y = ((s8)frame[offset--]) + screen_y;
        if (y < 0 || y > 239) {
            offset -= 2;
            continue;
        }
        oam.x = x;
        oam.y = y;
        u8 tile = frame[offset--];
        u8 attr = frame[offset--];
        oam.attr = attr | object.attribute;
        oam.tile = tile + object.tile_offset;
        sprite_slot += 4;
    }
}

template <size_t Mod>
prg_rom_2 static u8 mod_add(u8 l, u8 r) {
    u8 o = l + r;
    if (o >= Mod) {
        o -= Mod;
    }
    return o;
}

prg_rom_2 static void draw_hud() {
    const auto& player = objects[0];
    // TODO: don't hardcode these offsets
    constexpr u8 numeral_tile_offset = 0x2b;
    auto& catface_left = *reinterpret_cast<OAMData*>(&OAM_BUF[sprite_slot]);
    auto& catface_right = *reinterpret_cast<OAMData*>(&OAM_BUF[sprite_slot+4]);
    auto& hud_hp = *reinterpret_cast<OAMData*>(&OAM_BUF[sprite_slot+8]);

    catface_left.tile = numeral_tile_offset + (10 * 2);
    catface_right.tile = numeral_tile_offset + (11 * 2);
    hud_hp.tile = (player.hp << 1) + numeral_tile_offset;

    catface_left.attr = 0;
    catface_right.attr = 0;
    hud_hp.attr = 0;

    catface_left.x = 20;
    catface_right.x = 28;
    hud_hp.x = 40;

    catface_left.y = 24;
    catface_right.y = 24;
    hud_hp.y = 24;

    sprite_slot += 12;
}

namespace Sprite {

    prg_rom_2 void move_sprites_offscreen() {
        #pragma clang loop unroll(enable)
        for (s8 i = 0; i < 64; ++i) {
            auto& oam = reinterpret_cast<OAMData*>(&OAM_BUF)[i];
            oam.y = 0xff;
        }
    }

    prg_rom_2 void draw_objects() {
        sprite_slot = 0;
        const auto& player = objects[0];
    
        // Draw the player first to reserve their slot
        // Everything else can fight with flickering.
        
        // in order to fake the floating effect, if we are walking down
        // the draw the player second so the cube appears in front of the tail
        if (player.metasprite == Metasprite::KittyDown) {
            draw_object(1);
            draw_object(0);
        } else {
            draw_object(0);
            draw_object(1);
        }

        // OAM shuffle the rest of the sprites
        shuffle_offset = mod_add<OBJECT_COUNT>(shuffle_offset, 11);
        u8 original_offset = shuffle_offset;
        u8 count = OBJECT_COUNT;
        for (u8 i = original_offset; count > 0; --count, i = mod_add<OBJECT_COUNT>(i, 7)) {
            // we drew the player already and we guarantee they are in slot 0
            if (i == 0) {
                draw_hud();
                continue;
            } else if (i == 1) {
                continue;
            }

            draw_object(i);
        }
    }
}