
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

.section .prg_rom_fixed.atantables,"aR",@progbits

.globl octant_adjust
.globl atan_tab
.globl log2_tab

octant_adjust:
  .byte %00111111		;; x+,y+,|x|>|y|
  .byte %00000000		;; x+,y+,|x|<|y|
  .byte %11000000		;; x+,y-,|x|>|y|
  .byte %11111111		;; x+,y-,|x|<|y|
  .byte %01000000		;; x-,y+,|x|>|y|
  .byte %01111111		;; x-,y+,|x|<|y|
  .byte %10111111		;; x-,y-,|x|>|y|
  .byte %10000000		;; x-,y-,|x|<|y|

;;;;;;;; atan(2^(x/32))*128/pi ;;;;;;;;
atan_tab:
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$00,$00,$00
  .byte $00,$00,$00,$00,$00,$01,$01,$01
  .byte $01,$01,$01,$01,$01,$01,$01,$01
  .byte $01,$01,$01,$01,$01,$01,$01,$01
  .byte $01,$01,$01,$01,$01,$01,$01,$01
  .byte $01,$01,$01,$01,$01,$02,$02,$02
  .byte $02,$02,$02,$02,$02,$02,$02,$02
  .byte $02,$02,$02,$02,$02,$02,$02,$02
  .byte $03,$03,$03,$03,$03,$03,$03,$03
  .byte $03,$03,$03,$03,$03,$04,$04,$04
  .byte $04,$04,$04,$04,$04,$04,$04,$04
  .byte $05,$05,$05,$05,$05,$05,$05,$05
  .byte $06,$06,$06,$06,$06,$06,$06,$06
  .byte $07,$07,$07,$07,$07,$07,$08,$08
  .byte $08,$08,$08,$08,$09,$09,$09,$09
  .byte $09,$0a,$0a,$0a,$0a,$0b,$0b,$0b
  .byte $0b,$0c,$0c,$0c,$0c,$0d,$0d,$0d
  .byte $0d,$0e,$0e,$0e,$0e,$0f,$0f,$0f
  .byte $10,$10,$10,$11,$11,$11,$12,$12
  .byte $12,$13,$13,$13,$14,$14,$15,$15
  .byte $15,$16,$16,$17,$17,$17,$18,$18
  .byte $19,$19,$19,$1a,$1a,$1b,$1b,$1c
  .byte $1c,$1c,$1d,$1d,$1e,$1e,$1f,$1f

;;;;;;;; log2(x)*32 ;;;;;;;;
log2_tab:
  .byte $00,$00,$20,$32,$40,$4a,$52,$59
  .byte $60,$65,$6a,$6e,$72,$76,$79,$7d
  .byte $80,$82,$85,$87,$8a,$8c,$8e,$90
  .byte $92,$94,$96,$98,$99,$9b,$9d,$9e
  .byte $a0,$a1,$a2,$a4,$a5,$a6,$a7,$a9
  .byte $aa,$ab,$ac,$ad,$ae,$af,$b0,$b1
  .byte $b2,$b3,$b4,$b5,$b6,$b7,$b8,$b9
  .byte $b9,$ba,$bb,$bc,$bd,$bd,$be,$bf
  .byte $c0,$c0,$c1,$c2,$c2,$c3,$c4,$c4
  .byte $c5,$c6,$c6,$c7,$c7,$c8,$c9,$c9
  .byte $ca,$ca,$cb,$cc,$cc,$cd,$cd,$ce
  .byte $ce,$cf,$cf,$d0,$d0,$d1,$d1,$d2
  .byte $d2,$d3,$d3,$d4,$d4,$d5,$d5,$d5
  .byte $d6,$d6,$d7,$d7,$d8,$d8,$d9,$d9
  .byte $d9,$da,$da,$db,$db,$db,$dc,$dc
  .byte $dd,$dd,$dd,$de,$de,$de,$df,$df
  .byte $df,$e0,$e0,$e1,$e1,$e1,$e2,$e2
  .byte $e2,$e3,$e3,$e3,$e4,$e4,$e4,$e5
  .byte $e5,$e5,$e6,$e6,$e6,$e7,$e7,$e7
  .byte $e7,$e8,$e8,$e8,$e9,$e9,$e9,$ea
  .byte $ea,$ea,$ea,$eb,$eb,$eb,$ec,$ec
  .byte $ec,$ec,$ed,$ed,$ed,$ed,$ee,$ee
  .byte $ee,$ee,$ef,$ef,$ef,$ef,$f0,$f0
  .byte $f0,$f1,$f1,$f1,$f1,$f1,$f2,$f2
  .byte $f2,$f2,$f3,$f3,$f3,$f3,$f4,$f4
  .byte $f4,$f4,$f5,$f5,$f5,$f5,$f5,$f6
  .byte $f6,$f6,$f6,$f7,$f7,$f7,$f7,$f7
  .byte $f8,$f8,$f8,$f8,$f9,$f9,$f9,$f9
  .byte $f9,$fa,$fa,$fa,$fa,$fa,$fb,$fb
  .byte $fb,$fb,$fb,$fc,$fc,$fc,$fc,$fc
  .byte $fd,$fd,$fd,$fd,$fd,$fd,$fe,$fe
  .byte $fe,$fe,$fe,$ff,$ff,$ff,$ff,$ff