
.section .prg_rom_2,"ax",@progbits
.globl check_solid_collision_x

.set SIZE_OF_OBJECT, 23
.set SIZE_OF_SOLID_OBJECT, 7
.set OBJECT_COUNT, 16
.set SOLID_OBJECT_COUNT, 32

.set OBJECT_OFFSETOF_X_FR,          0  * OBJECT_COUNT
.set OBJECT_OFFSETOF_X_LO,          1  * OBJECT_COUNT
.set OBJECT_OFFSETOF_X_HI,          2  * OBJECT_COUNT
.set OBJECT_OFFSETOF_Y_FR,          3  * OBJECT_COUNT
.set OBJECT_OFFSETOF_Y_LO,          4  * OBJECT_COUNT
.set OBJECT_OFFSETOF_Y_HI,          5  * OBJECT_COUNT
 
.set OBJECT_OFFSETOF_HITBOX_X,      6  * OBJECT_COUNT
.set OBJECT_OFFSETOF_HITBOX_Y,      7  * OBJECT_COUNT
.set OBJECT_OFFSETOF_WIDTH,         8  * OBJECT_COUNT
.set OBJECT_OFFSETOF_HEIGHT,        9  * OBJECT_COUNT
.set OBJECT_OFFSETOF_COLLISION,     10 * OBJECT_COUNT

.set SOLID_OBJECT_OFFSETOF_WIDTH,   5  * SOLID_OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_HEIGHT,  6  * SOLID_OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_X_LO,    1  * SOLID_OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_X_HI,    2  * SOLID_OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_Y_LO,    3  * SOLID_OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_Y_HI,    4  * SOLID_OBJECT_COUNT
.set SOLID_OBJECT_OFFSETOF_STATE,   0  * SOLID_OBJECT_COUNT

.set obj_hitbox_width, __rc2
.set obj_x_lo, __rc3
.set obj_x_hi, __rc4
.set solid_state, __rc5
.set tmp, __rc6
.set filter, __rc7
.set obj_hitbox_height, __rc8
.set obj_y_lo, __rc9
.set obj_y_hi, __rc10
.set obj_offset, __rc11

; code based on this post from tokumaru
; https://forums.nesdev.org/viewtopic.php?p=285105#p285105
; dist = obj1x - obj2x
; if dist < -255 or dist > 255 return 0 //if using 16-bit positions
; if dist < 0 //obj1 is on the left
;   if abs(dist) < obj1width return 0
; else //obj2 is on the left
;   if dist < obj2width return 0
; end if

.globl check_solid_collision
check_solid_collision:
    sta filter
    ; Store obj.width - 1 in rc2 since this is used twice
    lda objects + OBJECT_OFFSETOF_WIDTH, x
    sta obj_hitbox_width
    lda objects + OBJECT_OFFSETOF_HEIGHT, x
    sta obj_hitbox_height

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
        ; If its not zero or -1, then the abs(distance) > 255 and skip this
        bne .Skip
    .NegativeXInRange:
        lda tmp
        ; if its 0 here, then its actually -256 so skip
        beq .Skip
        ; abs(dist) and compare with the width of the object
        eor #$ff
        ; carry is set because of cmp #$ff
        adc #0
        cmp solid_objects + SOLID_OBJECT_OFFSETOF_WIDTH, x
        bcs .Skip
        ;; X Overlap, time to check Y Collided!
        bcc .CheckY
    .PositiveXInRange:
        lda tmp
        ; if dist > 0 then check against player width
        cmp obj_hitbox_width
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
        lda tmp
        ; if its 0 here, then its actually -256 so skip
        beq .Skip
        eor #$ff
        ; carry is set because of cmp #$ff
        adc #0
        cmp solid_objects + SOLID_OBJECT_OFFSETOF_HEIGHT, x
        bcs .Skip
        bcc .Exit
    .PositiveYInRange:
        lda tmp
        cmp obj_hitbox_height
        bcc .Exit
        ; fallthrough
.Skip:
        dex
        bpl .Loop
    lda #0
    rts
.Exit:
    lda solid_state
    rts

.section ".zp.noinit"
.globl obj_collision_parameter
obj_collision_parameter:
    .space 6

.set PARAM_OFFSETOF_X_LO, 0
.set PARAM_OFFSETOF_X_HI, 1
.set PARAM_OFFSETOF_Y_LO, 2
.set PARAM_OFFSETOF_Y_HI, 3
.set PARAM_OFFSETOF_HITBOX_WIDTH, 4
.set PARAM_OFFSETOF_HITBOX_HEIGHT, 5
.set object_collide, __rc5

.section ".prg_rom_2.obj_collision"
.globl check_object_collision
check_object_collision:
    sta filter
    lda objects + OBJECT_OFFSETOF_COLLISION, x
    and filter
    sta object_collide
    ; Skip this object if we don't care about the hitbox for it
    beq .NoCollision

    ; perform 16 bit subtraction between the two X values
    lda objects + OBJECT_OFFSETOF_X_LO, x
    sec
    sbc obj_collision_parameter + PARAM_OFFSETOF_X_LO
    sta tmp
    lda objects + OBJECT_OFFSETOF_X_HI, x
    sbc obj_collision_parameter + PARAM_OFFSETOF_X_HI
    ; if the high byte is non-zero that means the two objects are more than
    ; 255 pixels away from each other so just ignore this check
    beq .PositiveXInRange2
    cmp #$ff
    ; If its not zero or -1, then the abs(distance) > 255 and skip this
    bne .NoCollision
.NegativeXInRange2:
    lda tmp
    ; if its 0 here, then its actually -256 so skip
    beq .NoCollision
    ; abs(dist) and compare with the width of the object
    eor #$ff
    ; carry is set because of cmp #$ff
    adc #0
    cmp objects + OBJECT_OFFSETOF_WIDTH, x
    bcs .NoCollision
    ;; X Overlap, time to check Y Collided!
    bcc .CheckY2
.PositiveXInRange2:
    lda tmp
    ; if dist > 0 then check against player width
    cmp obj_collision_parameter + PARAM_OFFSETOF_HITBOX_WIDTH
    bcs .NoCollision
.CheckY2:

    ; perform 16 bit subtraction between the two Y values
    lda objects + OBJECT_OFFSETOF_Y_LO, x
    sec
    sbc obj_collision_parameter + PARAM_OFFSETOF_Y_LO
    sta tmp
    lda objects + OBJECT_OFFSETOF_Y_HI, x
    sbc obj_collision_parameter + PARAM_OFFSETOF_Y_HI
    beq .PositiveYInRange2
    cmp #$ff
    bne .NoCollision
.NegativeYInRange2:
    lda tmp
    ; if its 0 here, then its actually -256 so skip
    beq .NoCollision
    eor #$ff
    ; carry is set because of cmp #$ff
    adc #0
    cmp objects + OBJECT_OFFSETOF_HEIGHT, x
    bcs .NoCollision
    bcc .YesCollision
.PositiveYInRange2:
    lda tmp
    cmp obj_collision_parameter + PARAM_OFFSETOF_HITBOX_HEIGHT
    bcc .YesCollision
    ; fallthrough
.NoCollision:
    lda #0
    rts
.YesCollision:
    lda object_collide
    rts

