
#include <mapper.h>
#include <neslib.h>
#include <peekpoke.h>

#include "common.hpp"
#include "dungeon_generator.hpp"
// #include "header/graphics_constants.hpp"
#include "header/sprites_constants.hpp"
#include "graphics.hpp"
#include "map_loader.hpp"
#include "map.hpp"
#include "object.hpp"
#include "rand.hpp"

// the sprite constants for the HUD are hardcoded for now
constexpr u8 hud_chr_count = 12 * 2;
constexpr u16 hud_chr_offset = hud_chr_count * 16;

struct SectionLookup {
    const char* nametable;
    const char* chr;
    const char* attribute;
    const char* palette;
    u8 mirroring;
};

// TODO: this table should be generated in python instead since it can really only happen in ASM
__attribute__((section(".prg_rom_1.section_lut"))) constexpr SectionLookup section_lut[7] = {
    {bottom_bin, room_updown_chr, bottom_attr, updown_palette, MIRROR_HORIZONTAL},
    {left_bin, room_leftright_chr, left_attr, leftright_palette, MIRROR_VERTICAL},
    {right_bin, room_leftright_chr, right_attr, leftright_palette, MIRROR_VERTICAL},
    {single_bin, room_single_chr, single_attr, single_palette, MIRROR_VERTICAL},
    {startdown_bin, room_start_chr, startdown_attr, start_palette, MIRROR_HORIZONTAL},
    {startup_bin, room_start_chr, startup_attr, start_palette, MIRROR_HORIZONTAL},
    {top_bin, room_updown_chr, top_attr, updown_palette, MIRROR_HORIZONTAL},
};

noinit Room room;
noinit Section lead;
noinit Section side;

static void read_map_room(u8 section_id) {
    // switch the bank to the bank with the save data
    set_chr_bank(3);

    // Really lame, but i don't have a better idea for this yet
    // just temporarily load the section into lead to get the room id
    u8 room_id = Dungeon::load_room_id_by_section(section_id);
    Dungeon::load_room_from_chrram(room_id);
    Dungeon::load_section_to_lead(room.lead_id);
    Dungeon::load_section_to_side(room.side_id);
}

// TODO: store the offset in RAM for the loaded objects
constexpr u8 get_tile_offset(const ObjectType& obj) {
    switch (obj) {
    case ObjectType::Player:
        return 0;
    case ObjectType::WeaponSphere:
    case ObjectType::WeaponPyramid:
    case ObjectType::WeaponDiamond:
    case ObjectType::WeaponCube:
        return kitty_chr_count + hud_chr_count;
    case ObjectType::Count:
        return 0;
    }
}

static void load_section(const Section& section) {

    u16 nmt_addr = ((u16)section.nametable) << 8;
    vram_adr(nmt_addr);
    const char* nametable = section_lut[static_cast<u8>(section.room_base)].nametable;
    donut_decompress(nametable);

    // load the attributes for this nametable into a buffer that we can update with the
    // objects as they are loaded
    std::array<u8, 64> attr_buffer;
    const char* attr = section_lut[static_cast<u8>(section.room_base)].attribute;
    for (u8 i = 0; i < 64; ++i) {
        attr_buffer[i] = attr[i];
    }

    // and then write out all the attributes
    vram_adr(nmt_addr + 0x3c0);
    vram_write(attr_buffer.data(), 64);
    
    set_prg_bank(CODE_BANK);

    // Load all objects for this side of the map
    u8 i = 1;
    for (const auto& obj : section.objects) {
        u8 slot_idx = i++;
        if (obj.id == ObjectType::Player) {
            slot_idx = 0;
        }
        auto slot = objects[slot_idx];
        if (i > OBJECT_COUNT) {
            break;
        }
        if ((slot.state & 0x80) == 0) {
            continue;
        }
        const auto init = object_init_data[(u8)obj.id];
        slot.state = init.state;
        slot.collision = init.collision;
        slot.metasprite = init.metasprite;
        slot.tile_offset = get_tile_offset(obj.id);
        slot.x = obj.x;
        slot.y = obj.y;
        slot.attribute = init.attribute;
        slot.hp = init.hp;
        slot.atk = init.atk;
        slot.hitbox.x = init.hitbox.x;
        slot.hitbox.y = init.hitbox.y;
        slot.hitbox.width = init.hitbox.width;
        slot.hitbox.height = init.hitbox.height;
    }
    // Load the basic solid objects from this style of map
    const auto& walls = room.scroll == ScrollType::Vertical ? updown_walls 
        : room.scroll == ScrollType::Horizontal ? leftright_walls : single_walls;
    i = 0;
    for (const auto& wall : walls) {
        if (wall.state == (CollisionType)0) {
            continue;
        }
        auto slot = solid_objects[i++];
        if (i > SOLID_OBJECT_COUNT) {
            break;
        }
        slot.state = wall.state;
        slot.x = room.x + wall.x;
        slot.y = room.y + wall.y;
        slot.width = wall.width;
        slot.height = wall.height;
    }

    set_prg_bank(GRAPHICS_BANK);
}

namespace MapLoader {

    void load_map(u8 section_id) {
        // TODO turn off DPCM if its playing
        // ppu_off();

        // switch to the 16kb bank that holds the level
        set_prg_bank(GRAPHICS_BANK);

        // clear out all old solid objects
        for (auto solid : solid_objects) {
            solid.state = (CollisionType)0;
        }

        // TODO save the previous room state when leaving?
        read_map_room(section_id);

        bg_chr_count = 0;
        bg_chr_offset = 0x0000;
        set_chr_bank(bg_chr_offset);
        vram_adr(0x00);
        const char* chr = section_lut[static_cast<u8>(lead.room_base)].chr;
        donut_decompress(chr);
        // bg_chr_offset += // todo we need to add the offsets to the section_lut

        // always add the kitty tile to the CHR
        sp_chr_count = 0;
        sp_chr_offset = 0x1000;
        vram_adr(sp_chr_offset);
        donut_decompress(&kitty_chr);
        sp_chr_count += kitty_chr_count;
        sp_chr_offset += kitty_chr_offset;

        // Load HUD font
        vram_adr(sp_chr_offset);
        donut_decompress(&hudfont_chr);
        sp_chr_count += hud_chr_count;
        sp_chr_offset += hud_chr_offset;

        // and load the weapons
        vram_adr(sp_chr_offset);
        donut_decompress(&weapons_chr);
        sp_chr_count += weapons_chr_count;
        sp_chr_offset += weapons_chr_offset;

        load_section(lead);

        if (side.room_id != Dungeon::NO_EXIT) {
            load_section(side);
        }

        // if we are using a vertical configuration, update the mirroring config
        u8 mirroring = section_lut[static_cast<u8>(lead.room_base)].mirroring;
        set_mirroring(mirroring);

        const char* palette = section_lut[static_cast<u8>(lead.room_base)].palette;
        pal_bg(palette);

        pal_bright(0);
        
        // restore the code bank
        set_prg_bank(CODE_BANK);
    }
}
