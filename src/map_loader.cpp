

#include <mapper.h>
#include <neslib.h>
#include <peekpoke.h>
#include <soa.h>

#include "common.hpp"
#include "dungeon_generator.hpp"
#include "header/sprites_constants.hpp"
#include "graphics.hpp"
#include "map_loader.hpp"
#include "map.hpp"
#include "object.hpp"
#include "rand.hpp"
#include "header/room_collision.hpp"

// the sprite constants for the HUD are hardcoded for now
constexpr u8 hud_chr_count = 12 * 2;
constexpr u16 hud_chr_offset = hud_chr_count * 16;

struct SectionLookup {
    const char* nametable;
    const char* chr;
    const char* attribute;
    const char* palette;
    u8 mirroring;
    u16 chr_offset;
    u8 chr_count;
};

#define SOA_STRUCT SectionLookup
#define SOA_MEMBERS MEMBER(nametable) MEMBER(chr) MEMBER(attribute) MEMBER(palette) MEMBER(mirroring) MEMBER(chr_offset) MEMBER(chr_count)
#include <soa-struct.inc>
extern const soa::Array<SectionLookup, static_cast<u8>(SectionBase::Count)> section_lut;

noinit Room room;
noinit Section lead;
noinit Section side;

noinit std::array<u8, 4> room_obj_chr_counts;

extern const u16 door_exit_offset[4];
extern const u8 door_exit_count[4];

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
constexpr u8 get_tile_offset(ObjectType obj) {
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

namespace RoomObject {
enum {
    DOOR_UP,
    DOOR_RIGHT,
    DOOR_DOWN,
    DOOR_LEFT,
};
}

struct SectionObjectRect {
    u8 x;
    u8 y;
    u8 width;
    u8 height;
};
#define SOA_STRUCT SectionObjectRect
#define SOA_MEMBERS MEMBER(x) MEMBER(y) MEMBER(width) MEMBER(height)
#include <soa-struct.inc>

struct SectionObjectLookup {
    const char* nametable;
    const char* chr;
    const char* attribute; // TODO
    u16 chr_offset;
    u8 chr_count;
    // SectionObjectRect pos;
};
#define SOA_STRUCT SectionObjectLookup
#define SOA_MEMBERS MEMBER(nametable) MEMBER(chr) MEMBER(attribute) MEMBER(chr_offset) MEMBER(chr_count)
#include <soa-struct.inc>
extern const soa::Array<SectionObjectLookup, 4> section_object_lut;

extern const soa::Array<SectionObjectRect, section_object_collision_total> section_collision_lut;

extern const soa::Array<SectionObjectRect, static_cast<u8>(SectionBase::Count) * 4> section_exit_lut;

// constexpr std::array<u8, 1> offset_lut = {1};
struct RoomObjectRect {
    s16 x;
    s16 y;
    u8 width;
    u8 height;
};


const u8 section_collision_offset[] 
__attribute__((section(".prg_rom_1.section_table"))) = {
    0,
    2,
    6,
    8,
    12,
    14,
    19,
    25
};

const u8 section_x_hi[static_cast<u8>(SectionBase::Count)] 
__attribute__((section(".prg_rom_1.wall_offset_x"))) = {
    0, 0, 1, 0, 0, 0, 0
};

const u8 section_y[static_cast<u8>(SectionBase::Count)] 
__attribute__((section(".prg_rom_1.wall_offset_y"))) = {
    240, 0, 0, 0, 240, 0, 0
};



static bool add_solid_wall(const SectionObjectRect& wall, SectionBase section) {
    if (solid_object_offset >= SOLID_OBJECT_COUNT) {
        return false;
    }
    if (wall.width == 0 && wall.height == 0) {
        return false;
    }
    auto slot = solid_objects[solid_object_offset++];
    // slot.state = wall.state;
    // TODO add this field to the export
    slot.state = CollisionType::Solid;
    slot.x = room.x + wall.x + (((u16)section_x_hi[(u8)section]) << 8);
    slot.y = room.y + wall.y + (u16)section_y[(u8)section];
    slot.width = wall.width;
    slot.height = wall.height;
    return true;
}

prg_rom_1 static void load_section(const Section& section) {
    // u8 section_base = static_cast<u8>(section.room_base);
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
    
    for (u8 i = section_collision_offset[static_cast<u8>(section.room_base)];
        i < section_collision_offset[static_cast<u8>(section.room_base) + 1]; ++i) {
        const auto wall = section_collision_lut[i];
        // if (wall.state == (CollisionType)0) {
        //     continue;
        // }
        add_solid_wall(wall, section.room_base);
    }

    // now load the exits
    for (u8 i = RoomObject::DOOR_UP; i <= RoomObject::DOOR_LEFT; ++i) {
        if ((section.exit[i] & 0x80) != 0) {
            // Theres an exit at this spot, so replace the chr tiles with the
            // the graphics for the new exit
            auto graphics = section_object_lut[i];
            // if we haven't loaded this exit type, copy it into chr
            if (room_obj_chr_counts[i] == 0) {
                room_obj_chr_counts[i] = bg_chr_count;
                vram_adr(bg_chr_offset);
                donut_decompress(graphics.chr);
                bg_chr_offset += graphics.chr_offset;
                bg_chr_count += graphics.chr_count;
            }
            // and now we can write the tile data to the nametable
            // auto exitlut = section_exit_lut[static_cast<u8>(section.room_base)];
            // auto exit = exitlut->exits[i];

            const auto wall = section_exit_lut[(u8)section.room_base*4 + i].get();
            u8 chr_offset = room_obj_chr_counts[i];
            u8 offset = 0;
            // Load the tiles
            for (u8 h=0; h < wall.height/8; ++h) {
                auto addr = nmt_addr | NTADR((u16)wall.x/8, (u16)(wall.y/8) + h);
                vram_adr(addr);
                for (u8 w=0; w < wall.width/8; ++w) {
                    vram_put(graphics.nametable.get()[offset] + chr_offset);
                    ++offset;
                }
            }
            // TODO: And now the attrs

            // There's an exit at this spot, so add the collision box
            // const auto wall = section_exit_lut[(u8)section.room_base*4 + i].get();
            add_solid_wall(wall, section.room_base);
        }
    }
    // For now, load all the 

    // and then write out all the attributes
    vram_adr(nmt_addr + 0x3c0);
    vram_write(attr_buffer.data(), 64);
    
    // set_prg_bank(CODE_BANK);

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

    // set_prg_bank(GRAPHICS_BANK);
}

// [[maybe_unused]] constexpr u8 section_base_to_mapstyle_lut[] = {

// };

namespace MapLoader {

    void load_map(u8 section_id) {
        // TODO turn off DPCM if its playing
        // ppu_off();

        for (u8 i=0; i < 4; ++i) {
            room_obj_chr_counts[i] = 0;
        }

        // switch to the 16kb bank that holds the level
        set_prg_bank(GRAPHICS_BANK);

        // clear out all old solid objects
        for (auto solid : solid_objects) {
            solid.state = (CollisionType)0;
        }

        // TODO save the previous room state when leaving
        read_map_room(section_id);

        bg_chr_count = 0;
        bg_chr_offset = 0x0000;
        set_chr_bank(0);
        vram_adr(0x00);
        u8 room_base = static_cast<u8>(lead.room_base);
        const char* chr = section_lut[room_base].chr;
        donut_decompress(chr);
        bg_chr_offset += section_lut[room_base].chr_offset;
        bg_chr_count += section_lut[room_base].chr_count;

        // Load the basic solid objects from this style of map
        // const auto& walls = room.scroll == ScrollType::Vertical ? updown_walls 
        //     : room.scroll == ScrollType::Horizontal ? leftright_walls : single_walls;
        // room_collision_lut[]

        // now load the collision data for this map
        // set_prg_bank(GRAPHICS_BANK);
        load_section(lead);

        if (side.room_id != Dungeon::NO_EXIT) {
            load_section(side);
        }

        // always add the kitty tile to the CHR
        // start on the chr offset at $1000
        sp_chr_count = 1;
        sp_chr_offset = 0x1000;
        vram_adr(sp_chr_offset);
        donut_decompress(&kitty_chr);
        sp_chr_count += kitty_chr_count;
        sp_chr_offset += kitty_chr_offset;

        // Load HUD font
        hud_tile_offset = sp_chr_count;
        vram_adr(sp_chr_offset);
        donut_decompress(&hudfont_chr);
        sp_chr_count += hud_chr_count;
        sp_chr_offset += hud_chr_offset;

        // and load the weapons
        vram_adr(sp_chr_offset);
        donut_decompress(&weapons_chr);
        sp_chr_count += weapons_chr_count;
        sp_chr_offset += weapons_chr_offset;

        // set_prg_bank(CODE_BANK);
        // if we are using a vertical configuration, update the mirroring config
        u8 mirroring = section_lut[room_base].mirroring;
        set_mirroring(mirroring);

        const char* palette = section_lut[room_base].palette;
        pal_bg(palette);

        pal_bright(0);
        
        // restore the code bank
        set_prg_bank(CODE_BANK);
    }
}
