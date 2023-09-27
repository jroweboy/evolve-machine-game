
#include <neslib.h>
#include <mapper.h>

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

asm(R"ASM(
.section .prg_rom_2,"ax",@progbits
.globl check_solid_collision_x

.set OBJECT_COUNT, 12
.set SOLID_OBJECT_COUNT, 20

.set OBJECT_OFFSETOF_HITBOX_X, 4 * OBJECT_COUNT
.set OBJECT_OFFSETOF_HITBOX_Y, 5 * OBJECT_COUNT
.set OBJECT_OFFSETOF_WIDTH, 6 * OBJECT_COUNT
.set OBJECT_OFFSETOF_HEIGHT, 7 * OBJECT_COUNT
.set OBJECT_OFFSETOF_X_LO, 0 * OBJECT_COUNT
.set OBJECT_OFFSETOF_X_HI, 1 * OBJECT_COUNT
.set OBJECT_OFFSETOF_Y_LO, 2 * OBJECT_COUNT
.set OBJECT_OFFSETOF_Y_HI, 3 * OBJECT_COUNT

.set SOLID_OBJECT_OFFSETOF_WIDTH, 5 * OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_HEIGHT, 6 * OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_X_LO, 1 * OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_X_HI, 2 * OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_Y_LO, 3 * OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_Y_HI, 4 * OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_STATE, 0 * OBJECT_COUNT

.set obj_hitbox_width, __rc2
.set obj_x_lo, __rc3
.set obj_x_hi, __rc4
.set solid_state, __rc5
.set tmp, __rc6
.set filter, __rc7
.set obj_hitbox_height, __rc8
.set obj_y_lo, __rc9
.set obj_y_hi, __rc10

check_solid_collision:
    sta filter
    ; Store obj.width - 1 in rc2 since this is used twice
    lda objects + OBJECT_OFFSETOF_WIDTH, x
    sta obj_hitbox_width

    ; Offset the X coord of the object by the hitbox X
    lda objects + OBJECT_OFFSETOF_X_LO, x
    clc
    adc objects + OBJECT_OFFSETOF_HITBOX_X, x
    sta obj_x_lo
    lda objects + OBJECT_OFFSETOF_X_HI, x
    adc #0
    sta obj_x_hi

    lda objects + OBJECT_OFFSETOF_Y_LO, x
    clc
    adc objects + OBJECT_OFFSETOF_HITBOX_Y, x
    sta obj_y_lo
    lda objects + OBJECT_OFFSETOF_Y_HI, x
    adc #0
    sta obj_y_hi

    ; Now go through all the solid object hitboxes looking for any matches
    ldx #SOLID_OBJECT_COUNT - 1
.Loop:
        lda solid_objects + SOLID_OBJECT_OFFSETOF_STATE, x
        and filter
        sta solid_state
        ; Skip this object if we don't care about the hitbox for it
        beq .Skip

        ; perform 16 bit subtraction between the two X values
        lda solid_objects + SOLID_OBJECT_OFFSETOF_X_LO, x
        sec
        sbc obj_x_lo
        sta tmp
        lda solid_objects + SOLID_OBJECT_OFFSETOF_X_HI, x
        sbc obj_x_hi
        ; if the high byte is non-zero that means the two objects are more than
        ; 255 pixels away from each other so just ignore this check
        beq .PositiveXInRange
        cmp #$ff
        ; If its not zero or one, then the abs(distance) > 255 and skip this
        bne .Skip
    .NegativeXInRange:
        ; flip the distance and compare with the width of the object
        eor #$ff
        ; carry is clear because of cmp #$ff
        adc #1
        cmp obj_hitbox_width
        bcs .Skip
        ;; X Overlap, time to check Y Collided!
        bcc .CheckY
    .PositiveXInRange:
        cmp solid_objects + SOLID_OBJECT_OFFSETOF_WIDTH, x
        bcs .Skip
    .CheckY:

        ; perform 16 bit subtraction between the two Y values
        lda solid_objects + SOLID_OBJECT_OFFSETOF_Y_LO, x
        sec
        sbc obj_y_lo
        sta tmp
        lda solid_objects + SOLID_OBJECT_OFFSETOF_Y_HI, x
        sbc obj_y_hi

        beq .PositiveYInRange
        cmp #$ff
        bne .Skip
    .NegativeYInRange:
        eor #$ff
        ; carry is clear because of cmp #$ff
        adc #1
        cmp obj_hitbox_width
        bcs .Skip
        bcc .Exit
    .PositiveYInRange:
        cmp solid_objects + SOLID_OBJECT_OFFSETOF_HEIGHT, x
        bcc .Exit
.Skip:
        dex
        bpl .Loop
    lda #0
    rts
.Exit:
    lda solid_state
    rts
)ASM");

prg_rom_2 static void move_player() {
    auto player = objects[0];
    auto pressed = pad_state(0);
    if (pressed & PAD_UP) {
        player.direction = Direction::Up;
        player.metasprite = 1;
        player.y--;
    } else if (pressed & PAD_DOWN) {
        player.direction = Direction::Down;
        player.metasprite = 2;
        player.y++;
    }
    u16 original_x = player.x;
    if (pressed & PAD_LEFT) {
        player.direction = Direction::Left;
        player.metasprite = 3;
        player.x--;
    } else if (pressed & PAD_RIGHT) {
        player.direction = Direction::Right;
        player.metasprite = 0;
        player.x++;
    }
    if (pressed & (PAD_DOWN | PAD_LEFT | PAD_RIGHT | PAD_UP)) {
        player.frame_counter--;
        if (player.frame_counter < 0) {
            player.frame_counter = 6;
            player.animation_frame = (player.animation_frame + 1) & 0b11;
        }
    }
    u8 collision = check_solid_collision(CollisionType::All, 0);
    if (collision > 0) {
        player.x = original_x;
    }
}

prg_rom_2 static void scroll_screen() {
    if (room.scroll == ScrollType::Single) {
        return;
    }
    // This room has scroll in it
    auto player = objects[0];
    if (room.scroll == ScrollType::Vertical) {
        s16 screen_pos_y = player.y - room.y - view.y;
        if (screen_pos_y < 0 || screen_pos_y > 239) {
            return;
        }
        if (screen_pos_y > 0x88 && view.y != 240) {
            view.y++;
        }
        if (screen_pos_y < 0x68 && view.y != 0) {
            view.y--;
        }
    } else {
        s16 screen_pos_x = player.x - room.x - view.x;
        if (screen_pos_x < 0 || screen_pos_x > 255) {
            return;
        }
        if (screen_pos_x > 0x90 && view.x != 255) {
            view.x++;
        }
        if (screen_pos_x < 0x70 && view.x != 0) {
            view.x--;
        }
    }
    scroll(view.x, view.y);
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
    auto player = objects[0];
    player.metasprite = 0;
    player.state = (State)0;
    player.x = room.x + 100;
    player.y = room.y + 100;
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

    move_player();
    scroll_screen();

    // Skip drawing sprites this frame if we lagged on the previous frame.
    // This should help us get caught up if we process too much in one frame
    if (!lag_frame) {
        Sprite::move_sprites_offscreen();
        Sprite::draw_objects();
    }
    lag_frame = frame != FRAME_CNT1;
}
}