
#include <neslib.h>

#include "sprite_render.hpp"
#include "object.hpp"

struct OAMData {
    u8 y;
    u8 tile;
    u8 attr;
    u8 x;
};

struct Metasprite {
    u8 size;
    u8 frames;
    OAMData oam[];
};

static u8 shuffle_offset;

extern const Metasprite player_metasprite;
extern const Metasprite walker_metasprite;

static const Metasprite* metasprite_table[static_cast<u8>(ObjectType::Count)] = {
    &player_metasprite,
    &walker_metasprite,
};

static u8 sprite_slot;

extern char OAM_BUF[];
static OAMData* OAM = &*reinterpret_cast<OAMData*>(&OAM_BUF);

static void draw_object(u8 id) {
    const auto& object = objects[id];

    if (object.state == State::Dead) {
        return;
    }

    // convert x/y positions to screen coords
    u8 screen_x = object.x - view.x;
    u8 screen_y = object.y - view.y;

    auto spr = metasprite_table[object.metasprite + object.direction];
    auto size = spr->size;
    auto frames = spr->frames;

    auto frame = &spr->oam[object.animation_frame * size];
    for (u8 i = 0; i < size; ++i) {
        frame = frame + i;
        auto oam = OAM[sprite_slot + i];
        oam.x = screen_x + frame->x;
        oam.y = screen_y + frame->y;
        oam.attr = frame->attr;
        oam.tile = frame->tile;
    }
}

namespace Sprite {

    void init() {
        // TODO not needed?
        sprite_slot = 0;
    }

    void draw_objects() {
        
        // Draw the player first to reserve their slot
        // Everything else can fight with flickering.
        draw_object(0);

        // OAM shuffle the rest of the sprites
        // 17 and 23 are co prime with 24
        shuffle_offset += 17;
        u8 original_offset = shuffle_offset;
        for (u8 i = shuffle_offset; i != original_offset; i = (i + 23) % 24) {
            // we drew the player already and we guarantee they are in slot 0
            if (i == 0) {
                continue;
            }
            draw_object(i);
        }
    }
}