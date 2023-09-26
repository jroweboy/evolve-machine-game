
#include <mapper.h>
#include <neslib.h>
#include <soa.h>

#include "sprite_render.hpp"
#include "object.hpp"

struct OAMData {
    u8 y;
    u8 tile;
    u8 attr;
    u8 x;
};

extern const u8 kitty_walk_right[];
extern const u8 kitty_walk_up[];
extern const u8 kitty_walk_down[];
extern const u8 kitty_walk_left[];

__attribute__((section(".prg_rom_2.metasprite_table"))) static const u8* metasprite_table[] = {
    kitty_walk_right,
    kitty_walk_up,
    kitty_walk_down,
    kitty_walk_left,
};

static u8 shuffle_offset;
static u8 sprite_slot;

extern u8 OAM_BUF[256];
// volatile OAMData* OAM = &*reinterpret_cast<volatile OAMData*>(&OAM_BUF);

__attribute__((section(".prg_rom_2"))) static void draw_object(u8 id) {
    const auto& object = objects[id];

    // If the high bit of the object state is 1 then its not going to be drawn so skip it
    if ((object.state & State::Hidden) != 0x0) {
        return;
    }

    // TODO: convert x/y positions to screen coords
    u8 screen_x = object.x - view.x;
    u8 screen_y = object.y - view.y;

    auto frame = metasprite_table[object.metasprite];
    // auto addr = reinterpret_cast<const u16*>(spr)[object.direction];
    // auto frame = reinterpret_cast<u8*>(addr);
    auto offset = frame[object.animation_frame];
    while (frame[offset] != 0x7f) {
        auto& oam = *reinterpret_cast<OAMData*>(&OAM_BUF[sprite_slot]);
        s8 x = frame[offset--];
        s8 y = frame[offset--];
        u8 tile = frame[offset--];
        u8 attr = frame[offset--];
        oam.x = x + object.x;
        oam.y = y + object.y;
        oam.attr = attr;
        oam.tile = tile + object.tile_offset;
        sprite_slot += 4;
    }
}

template <size_t Mod>
__attribute__((section(".prg_rom_2"))) static u8 mod_add(u8 l, u8 r) {
    u8 o = l + r;
    if (o >= Mod) {
        o -= Mod;
    }
    return o;
}

__attribute__((section(".prg_rom_2")))
static void draw_hud() {
    // TODO
}

namespace Sprite {

    __attribute__((section(".prg_rom_2"))) void move_sprites_offscreen() {
        #pragma clang loop unroll(enable)
        for (s8 i = 0; i < 64; ++i) {
            auto& oam = reinterpret_cast<OAMData*>(&OAM_BUF)[i];
            oam.y = 0xff;
        }
    }

    void draw_objects() {
        sprite_slot = 0;
    
        // Draw the player first to reserve their slot
        // Everything else can fight with flickering.
        draw_object(0);

        // OAM shuffle the rest of the sprites
        // 17 and 23 are co prime with 24
        shuffle_offset = mod_add<OBJECT_COUNT>(shuffle_offset, 7);
        u8 original_offset = shuffle_offset;
        u8 i = mod_add<OBJECT_COUNT>(shuffle_offset, 11);
        for (; i != original_offset; i = mod_add<OBJECT_COUNT>(i, 11)) {
            // we drew the player already and we guarantee they are in slot 0
            if (i == 0) {
                draw_hud();
                continue;
            }
            draw_object(i);
        }
    }
}