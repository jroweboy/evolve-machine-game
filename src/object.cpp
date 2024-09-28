
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
#include <peekpoke.h>
#include <soa.h>

struct ObjectInitData {
    Metasprite metasprite;
    Hitbox hitbox;
    State state;
    Speed speed;
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

__attribute__((leaf)) u8 arctan2(u8 slot1, u8 slot2) {
    u8 result;
    u8 scratch1;
    u8 scratch2;
    u8 scratch3;
    __attribute__((leaf)) __asm__ volatile(R"ASM(

.set OBJECT_COUNT, 16
OBJECT_X_FRAC     = 0  * OBJECT_COUNT
OBJECT_X_LO       = 1  * OBJECT_COUNT
OBJECT_X_HI       = 2  * OBJECT_COUNT
OBJECT_Y_FRAC     = 3  * OBJECT_COUNT
OBJECT_Y_LO       = 4  * OBJECT_COUNT
OBJECT_Y_HI       = 5  * OBJECT_COUNT

    tay
    sec
    lda objects + OBJECT_X_LO,x
    sbc objects + OBJECT_X_LO,y
    sta %[xdiff]
    lda objects + OBJECT_X_HI,x
    sbc objects + OBJECT_X_HI,y
    lda %[xdiff]
    bcs 1f
        eor #$ff
        sta %[xdiff]
1:
    rol %[octant]

    lda objects + OBJECT_Y_LO,x
    sbc objects + OBJECT_Y_LO,y
    sta %3
    lda objects + OBJECT_Y_HI,x
    sbc objects + OBJECT_Y_HI,y
    lda %3
    bcs 1f
        eor #$ff
1:
    tay
    rol %[octant]

    ldx %[xdiff]
    lda log2_tab,x
    sbc log2_tab,y
    bcc 1f
        eor #$ff
1:
    tax
    lda %[octant]
    rol
    and #7
    tay
    lda atan_tab,x
    eor octant_adjust,y
    lsr
    lsr
    lsr
    lsr
)ASM"
    :"+a"(result), [xdiff]"=r"(scratch1), [ydiff]"=r"(scratch2), [octant]"=r"(scratch3)
    : "a"(slot1), "x"(slot2)
    : "y", "p"
    );
    return result;
}

typedef union {
  uint16_t value;
  struct {
    uint8_t lo, hi;
  };
} Word;

__attribute__((leaf)) s16 distance(u8 slot1, u8 slot2) {
    Word result;
    u8 scratch1;u8 scratch2;u8 scratch3;
    u8 scratch4;u8 scratch5;u8 scratch6;
    __attribute__((leaf)) __asm__ volatile(R"ASM(
.set OBJECT_COUNT, 16
OBJECT_X_FRAC     = 0  * OBJECT_COUNT
OBJECT_X_LO       = 1  * OBJECT_COUNT
OBJECT_X_HI       = 2  * OBJECT_COUNT
OBJECT_Y_FRAC     = 3  * OBJECT_COUNT
OBJECT_Y_LO       = 4  * OBJECT_COUNT
OBJECT_Y_HI       = 5  * OBJECT_COUNT

    tay
    sec
    lda objects + OBJECT_Y_LO,x
    sbc objects + OBJECT_Y_LO,y
    sta %[ylo]
    lda objects + OBJECT_Y_HI,x
    sbc objects + OBJECT_Y_HI,y
    sta %[yhi]
    bcs 1f
        eor #$ff
        sta %[yhi]
        lda %[ylo]
        eor #$ff
        adc #1
        sta %[ylo]
1:
    sec
    lda objects + OBJECT_X_LO,x
    sbc objects + OBJECT_X_LO,y
    sta %[xlo]
    lda objects + OBJECT_X_HI,x
    sbc objects + OBJECT_X_HI,y
    bcs 1f
        sta %[xhi]
            lda %[xlo]
            eor #$ff
            adc #1
            sta %[xlo]
        lda %[xhi]
        eor #$ff
1:
    sta %[xhi]
    cmp %[yhi]
    bcc 2f
    bne 1f
    lda %[xlo]
    cmp %[ylo]
    bcc 2f
1:
        ; Y < X
        lda %[ylo]
        asl
        sta %[outlo]
        lda %[yhi]
        rol
        sta %[outhi]
        clc
        lda %[ylo]
        adc %[outlo]
        sta %[outlo]
        lda %[yhi]
        adc %[outhi]
        lsr
        ror %[outlo]
        lsr
        ror %[outlo]
        lsr
        ror %[outlo]
        sta %[outhi]
        lda %[outlo]
        clc
        adc %[xlo]
        tax
        lda %[yhi]
        adc %[outhi]
        jmp Exit
2:
    ; X < Y
    ; multiply by 3
    lda %[xlo]
    asl
    sta %[outlo]
    lda %[xhi]
    rol
    sta %[outhi]
    clc
    lda %[xlo]
    adc %[outlo]
    sta %[outlo]
    lda %[xhi]
    adc %[outhi] ; divide by 8
    lsr
    ror %[outlo]
    lsr
    ror %[outlo]
    lsr
    ror %[outlo]
    sta %[outhi]
    lda %[outlo]
    clc
    adc %[ylo]
    tax
    lda %[yhi]
    adc %[outhi]
Exit:
)ASM"
    :"+a"(result.hi), "+x"(result.lo),
     [ylo]"=r"(scratch1), [yhi]"=r"(scratch2), [xlo]"=r"(scratch3),
     [xhi]"=r"(scratch4), [outlo]"=r"(scratch5), [outhi]"=r"(scratch6)
    : "a"(slot1), "x"(slot2)
    : "y", "p"
);
    return (s16)result.value;
}

asm(R"ASM(
.section .prg_rom_fixed.speed_table,"aR",@progbits
.globl speed_table
speed_table:
    .incbin "gen/header/speed_table.bin"
)ASM");

prg_rom_2 noinline XYMagnitude get_angular_speed(Speed speed, u8 angle) {
    Word outx;
    Word outy;
    u8 tmp = 0;
    
    asm(R"ASM(
    cmp #8
    bcc 2f
        cmp #$ff
        bne 1f
            lda #0
            sta %[ylo]
            sta %[yhi]
            sta %[xlo]
            sta %[xhi]
            beq Exit
1:
        ; Speed table has 8 entries spread out so we can double them
        ; if its past the 8th entry, use the doubled version of the speed
        sta %[mult2]
2:
    stx %[ylo]
    ; speed * 32
    ror
    ror
    ror
    ror
    clc
    adc %[ylo]
    tay
    lda speed_table + 0*32*8, y
    sta %[ylo]
    lda speed_table + 1*32*8, y
    sta %[yhi]
    lda speed_table + 2*32*8, y
    sta %[xlo]
    lda speed_table + 3*32*8, y
    sta %[xhi]
    ; account for the doubled speeds here
    lda %[mult2]
    beq Exit
        asl %[ylo]
        rol %[yhi]
        asl %[xlo]
        rol %[xhi]
Exit:
)ASM"
    : [xlo]"=r"(outx.lo), [xhi]"=r"(outx.hi),
      [ylo]"=r"(outy.lo), [yhi]"=r"(outy.hi),
      [mult2]"+r"(tmp)
    : "a"(speed), "x"(angle)
    : "y", "p"
    );
    return {fs8_8(outx.hi, outx.lo), fs8_8(outy.hi, outy.lo)};
}

namespace Objects {

prg_rom_2 void move_object_offscr_check(u8 slot) {
    auto obj = objects[slot];
    auto speed = get_angular_speed(obj.speed, obj.angle);
    obj.y = obj->y + speed.y;
    if (obj.y->as_i() < room.y || obj.y->as_i() > room_bounds_y) {
        obj.state = State::Dead;
        return;
    }
    obj.x = obj->x + speed.x;
    if (obj.x->as_i() < room.x || obj.x->as_i() > room_bounds_x) {
        obj.state = State::Dead;
    }
}

prg_rom_2 void move_object_with_solid_collision(u8 slot) {
    auto obj = objects[slot];
    auto init = object_init_data[(u8)obj->type];
    u16 original_y = obj.y->as_i();
    u8 orig_direction = obj.direction;
    // auto speed = multidirection_lut[player->direction] 
    //     ? speed_table[(u8)player->speed].xy.get()
    //     : speed_table[(u8)player->speed].v.get();
    // DEBUGGER(speed.as_i());
    if (orig_direction & Direction::Up) {
        obj.metasprite = (Metasprite)((u8)init->metasprite + 0);
    } else if (orig_direction & Direction::Down) {
        obj.metasprite = (Metasprite)((u8)init->metasprite + 2);
    }
    u16 original_x = obj.x->as_i();
    if (pressed & PAD_LEFT) {
        player.direction |= Direction::Left;
        player.metasprite = Metasprite::KittyLeft;
    } else if (pressed & PAD_RIGHT) {
        player.direction |= Direction::Right;
        player.metasprite = Metasprite::KittyRight;
    }

    if (player.direction != 0) {
        player.angle = direction_to_angle_lut[player->direction];
        player.speed = Speed::s1_20;
    }

    auto speed = get_angular_speed(player->speed, player->angle);
    player.x = player->x + speed.x;
    if (player.x->as_i() != original_x) {
        u8 collision = check_solid_collision(CollisionType::All, 0);
        if (collision > 0) {
            player.x = original_x;
        }
    }
    player.y = player->y + speed.y;
    if (player.y->as_i() != original_y) {
        u8 collision = check_solid_collision(CollisionType::All, 0);
        if (collision > 0) {
            player.y = original_y;
        }
    }

    if (pressed & (PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_UP)) {
        player.frame_counter--;
        if (player.frame_counter < 0) {
            player.frame_counter = 6;
            player.animation_frame = (player.animation_frame + 1) & 0b11;
        }
    } else {
        player.direction = orig_direction;
    }
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
    for (u8 i=2; i < OBJECT_COUNT; ++i) {
        // auto obj = ;
        if ((objects[i].state & 0x80) != 0) {
            continue;
        }

        switch (objects[i].type) {
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
            move_object_offscr_check(i);
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
    slot.speed = init->speed;
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

constexpr static bool ismulti(u8 a) {
    if ((a & Direction::Up) != 0 && ((a & Direction::Left) != 0 || (a & Direction::Right) != 0)) {
        return true;
    }
    if ((a & Direction::Down) != 0 && 0 != ((a & Direction::Left) != 0 || (a & Direction::Right) != 0)) {
        return true;
    }
    return false;
}

constexpr static u8 dir2angle(u8 a) {
    if ((a & (Direction::Down | Direction::Right)) == (Direction::Down | Direction::Right)) {
        return 4;
    }
    if ((a & (Direction::Down | Direction::Left)) == (Direction::Down | Direction::Left)) {
        return 12;
    }
    if ((a & (Direction::Up | Direction::Left)) == (Direction::Up | Direction::Left)) {
        return 20;
    }
    if ((a & (Direction::Up | Direction::Right)) == (Direction::Up | Direction::Right)) {
        return 28;
    }
    if ((a & Direction::Up) == Direction::Up) {
        return 24;
    }
    if ((a & Direction::Down) == Direction::Down) {
        return 8;
    }
    if ((a & Direction::Left) == Direction::Left) {
        return 16;
    }
    return 0;
}

__attribute__((section(".prg_rom_fixed.multidirection_lut")))
constexpr const bool multidirection_lut[16] = {
    ismulti(0),ismulti(1),ismulti(2),ismulti(3),
    ismulti(4),ismulti(5),ismulti(6),ismulti(7),
    ismulti(8),ismulti(9),ismulti(10),ismulti(11),
    ismulti(12),ismulti(13),ismulti(14),ismulti(15),
};
__attribute__((section(".prg_rom_fixed.direction_to_angle_lut")))
constexpr const u8 direction_to_angle_lut[16] = {
    dir2angle(0),dir2angle(1),dir2angle(2),dir2angle(3),
    dir2angle(4),dir2angle(5),dir2angle(6),dir2angle(7),
    dir2angle(8),dir2angle(9),dir2angle(10),dir2angle(11),
    dir2angle(12),dir2angle(13),dir2angle(14),dir2angle(15),
};

__attribute__((section(".prg_rom_2.object_init_data")))
constexpr soa::Array<const ObjectInitData, (u8)ObjectType::Count> object_init_data = {
    {
        .metasprite = Metasprite::KittyLeft,
        .hitbox = { .x = 0, .y = 8, .width = 8, .height = 8},
        .state = State::Normal,
        .speed = Speed::s1_10,
        .collision = 0,
        .attribute = 0,
        .hp = 3,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCube,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .speed = Speed::None,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamond,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .speed = Speed::None,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramid,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .speed = Speed::None,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphere,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::GroundedWeapon,
        .speed = Speed::None,
        .collision = CollisionType::Pickup,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCubeAtk1,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s1_65,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCubeAtk2,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s1_65,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponCubeAtk3,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s1_65,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamondAtk1,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s2_20,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamondAtk2,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s2_20,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponDiamondAtk3,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s2_20,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramidAtk1,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s2_40,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramidAtk2,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s2_40,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponPyramidAtk3,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s2_40,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphereAtk1,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s1_40,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphereAtk2,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s1_40,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::WeaponSphereAtk3,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s1_40,
        .collision = CollisionType::Bullet,
        .attribute = 1,
        .hp = 0,
        .atk = 0,
    },
    {
        .metasprite = Metasprite::PidgeyLeft,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s1_00,
        .collision = CollisionType::Enemy,
        .attribute = 1,
        .hp = 50,
        .atk = 5,
    },
    {
        .metasprite = Metasprite::ArmadilloLeft,
        .hitbox = { .x = 0, .y = 0, .width = 16, .height = 16},
        .state = State::Normal,
        .speed = Speed::s1_00,
        .collision = CollisionType::Enemy,
        .attribute = 1,
        .hp = 80,
        .atk = 5,
    },
};


// constexpr soa::Array<SpeedTable, (u8)Speed::Count> speed_table = {
//     {1.0_8_8, 1.0_8_8, 0.77_8_8},
//     {1.2_8_8, 1.2_8_8, 1.69_8_8},
// };

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
