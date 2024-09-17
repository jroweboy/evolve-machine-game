
// TODO: move this to a spreadsheet or something i dunno.

// #include "object.hpp"
// #include "common.hpp"

// constexpr u8 WALL_SHORT = 16;
// constexpr u8 WALL_LONG = 100;

#include "object.hpp"
#include "common.hpp"
#include "game.hpp"
#include "map.hpp"
#include <fixed_point.h>
#include <mapper.h>
#include <soa.h>

struct ObjectInitData {
    Metasprite metasprite;
    Hitbox hitbox;
    State state;
    u8 collision;
    u8 attribute;
    s8 hp;
    u8 atk;
};
#define SOA_STRUCT ObjectInitData
#define SOA_MEMBERS \
    MEMBER(metasprite) MEMBER(hitbox) MEMBER(hp) MEMBER(atk) MEMBER(collision) MEMBER(attribute) \
    MEMBER(state)
#include <soa-struct.inc>

extern const soa::Array<const ObjectInitData, (u8)ObjectType::Count> object_init_data;


namespace Objects {

prg_rom_2 void move_object_offscr_check(u8 slot, fu8_8 x, fu8_8 y) {
    DEBUGGER();
    auto obj = objects[slot];
    if (obj->direction & Direction::Up) {
        obj.y = obj->y - y;
    } else if (obj->direction & Direction::Down) {
        obj.y = obj->y + y;
    }
    if (obj.y->as_i() < room.y || obj.y->as_i() > room_bounds_y) {
        obj.state = State::Dead;
    }
    if (obj->direction & Direction::Left) {
        obj.x = obj->x - x;
    } else if (obj->direction & Direction::Right) {
        obj.x = obj->x + x;
    }
    if (obj.x->as_i() < room.x || obj.x->as_i() > room_bounds_x) {
        obj.state = State::Dead;
    }
    DEBUGGER();
}

// prg_rom_2 void cube_atk(u8 slot, u8 level) {
//     // const auto player = objects[0];
//     // auto atk = objects[slot];
//     switch (level) {
//     case 1:
//         // for (u8 i = 2; i < OBJECT_COUNT; i++) {
//         //     auto enemy = objects[i];
//         //     if ((enemy.collision & CollisionType::Enemy) && enemy->state != State::Dead) {
//         //         auto enemy_x = (enemy->x.as_i() + enemy->hitbox.x) & 0xf8;
//         //         auto compared = enemy_x - (atk->x.as_i() & 0xf8);
//         //         if (compared >= 0 && compared < enemy->hitbox.width) {
//         //             atk.direction = (enemy->y.as_i() > atk->y.as_i()) ? Direction::Down : Direction::Up;
//         //             break;
//         //         }
//         //         auto enemy_y = (enemy->y.as_i() + enemy->hitbox.y) & 0xf8;
//         //         compared = enemy_y - (atk->y.as_i() & 0xf8);
//         //         if (compared >= 0 && compared < enemy->hitbox.height) {
//         //             atk.direction = (enemy->x.as_i() > atk->x.as_i()) ? Direction::Right : Direction::Left;
//         //             break;
//         //         }
//         //     }
//         // }
//         move_object_offscr_check(slot, 1.0_8_8, 1.0_8_8);
//         break;
//     case 2:
//         break;
//     case 3:
//         break;
//     }
// }

prg_rom_2 void core_loop() {
    for (int i=2; i < OBJECT_COUNT; ++i) {
        auto obj = objects[i];
        if ((obj->state & 0x80) != 0) {
            continue;
        }
        switch (obj->type) {
        case ObjectType::Armadillo:
        case ObjectType::Pidgey:
            break;
        case ObjectType::WeaponCubeAtk1:
            // cube_atk(i, 1);
            // break;
        case ObjectType::WeaponCubeAtk2:
            // cube_atk(i, 2);
            // break;
        case ObjectType::WeaponCubeAtk3:
            // cube_atk(i, 3);
            // break;
        case ObjectType::WeaponDiamondAtk1:
        case ObjectType::WeaponDiamondAtk2:
        case ObjectType::WeaponDiamondAtk3:
        case ObjectType::WeaponPyramidAtk1:
        case ObjectType::WeaponPyramidAtk2:
        case ObjectType::WeaponPyramidAtk3:
        case ObjectType::WeaponSphereAtk1:
        case ObjectType::WeaponSphereAtk2:
        case ObjectType::WeaponSphereAtk3:
            move_object_offscr_check(i, 1.7_8_8, 1.7_8_8);
            break;

        case ObjectType::Count:
        case ObjectType::None:
        case ObjectType::Player:
        case ObjectType::WeaponCube:
        case ObjectType::WeaponDiamond:
        case ObjectType::WeaponPyramid:
        case ObjectType::WeaponSphere:
            continue;
        }
    }
}


prg_rom_2 noinline u8 load_object_b2(ObjectType o) {
    u8 slot_idx = (o == ObjectType::Player) ? 0 : 2;
    while (slot_idx < OBJECT_COUNT) {
        auto slot = objects[slot_idx];
        if ((slot->state & State::Hidden) != 0) {
            break;
        }
        slot_idx++;
    }
    if (slot_idx >= OBJECT_COUNT)
    {
        return 0xff;
    }
    const auto init = object_init_data[(u8)o];
    auto slot = objects[slot_idx];
    slot.state = init->state;
    slot.collision = init->collision;
    slot.metasprite = init->metasprite;
    slot.attribute = init->attribute;
    slot.hp = init->hp;
    slot.atk = init->atk;
    slot.hitbox.x = init->hitbox.x;
    slot.hitbox.y = init->hitbox.y;
    slot.hitbox.width = init->hitbox.width;
    slot.hitbox.height = init->hitbox.height;
    slot.type = o;
    return slot_idx;
}

prg_fixed noinline u8 load_object(ObjectType o) {
    auto bank = get_prg_bank();
    set_prg_bank(2);
    auto ret = load_object_b2(o);
    set_prg_bank(bank);
    return ret;
}
}

__attribute__((section(".prg_rom_2.object_init_data")))
constexpr soa::Array<const ObjectInitData, (u8)ObjectType::Count> object_init_data = {
    {
        .metasprite = Metasprite::KittyLeft,
        .hitbox = { .x = 0, .y = 8, .width = 8, .height = 8},
        .state = State::Normal,
        .collision = 0,
        .attribute = 0,
        .hp = 3,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCube,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamond,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramid,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphere,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCubeAtk1,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCubeAtk2,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCubeAtk3,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamondAtk1,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamondAtk2,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamondAtk3,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramidAtk1,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramidAtk2,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramidAtk3,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphereAtk1,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphereAtk2,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphereAtk3,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::PidgeyLeft,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Enemy,
        .attribute = 1,
        .hp = 50,
        .atk = 5,
    },
    {
        .metasprite = Metasprite::ArmadilloLeft,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .collision = CollisionType::Enemy,
        .attribute = 1,
        .hp = 80,
        .atk = 5,
    },
};


constexpr soa::Array<SpeedTable, (u8)Speed::Count> speed_table = {
    {1.0_8_8, 1.0_8_8, 0.77_8_8},
    {1.2_8_8, 1.2_8_8, 1.69_8_8},
    {1.0_8_8, 1.0_8_8, 0.77_8_8},
};

// prg_rom_2 SolidObject updown_walls[ROOM_WALL_COUNT] = {
//     // top left corner
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = WALL_LONG, .height = WALL_SHORT,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = WALL_SHORT, .height = WALL_LONG,
//     },
//     // top right corner
//     {
//         .state = CollisionType::Solid,
//         .x = 256 - WALL_LONG, .y = 0, .width = WALL_LONG, .height = WALL_SHORT,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 256 - WALL_SHORT, .y = 0, .width = WALL_SHORT, .height = WALL_LONG,
//     },
//     // bot left corner
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 480 - WALL_SHORT, .width = WALL_LONG, .height = WALL_SHORT,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 480 - WALL_LONG, .width = WALL_SHORT, .height = WALL_LONG,
//     },
//     // bot right corner
//     {
//         .state = CollisionType::Solid,
//         .x = 256 - WALL_LONG, .y = 480 - WALL_SHORT, .width = WALL_LONG, .height = WALL_SHORT,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 256 - WALL_SHORT, .y = 480 - WALL_LONG, .width = WALL_SHORT, .height = WALL_LONG,
//     },
// };

// prg_rom_2 SolidObject leftright_walls[ROOM_WALL_COUNT] = {
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = 100, .height = 16,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = 16, .height = 100,
//     }
// };

// prg_rom_2 SolidObject single_walls[ROOM_WALL_COUNT] = {
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = 100, .height = 16,
//     },
//     {
//         .state = CollisionType::Solid,
//         .x = 0, .y = 0, .width = 16, .height = 100,
//     }
// };
