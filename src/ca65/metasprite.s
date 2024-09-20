

;;;;
; Reserves a spot in the metasprite table for this named constant.
; This is used internally when making a new metasprite, but can also
; be used to make your own metasprite. Just reserve a slot with the name
; of the metasprite and 
.importzp FRAME_CNT1

METASPRITES_COUNT .set 0

.macro IncludeMetasprite Name, Id
.ifndef .ident( .sprintf("METASPRITE_%s_DATA_%d", Name, Id) )
  .incbin .sprintf("gen/sprites/%s_metasprite_%d.bin", Name, Id)
  .ident( .sprintf("METASPRITE_%s_DATA_%d", Name, Id) ):
.endif
.endmacro

.macro MetaspriteOffset Base, Name, Id, Frame
.local Data
Data = .ident( .sprintf("METASPRITE_%s_DATA_%d", Name, Id) )
.assert (Data - Base - 1) < 256, error, .sprintf("Metasprite %s frame %d offset is > 256", Name, Frame)
.byte Data - Base - 1
.endmacro

.macro MetaspriteReserve Name
.ident( .sprintf("METASPRITE_%s", Name) ) = METASPRITES_COUNT
METASPRITES_COUNT .set METASPRITES_COUNT + 1
.endmacro

.macro MetaspriteAnimation Name, Animation, F1, F2, F3, F4, F5, F6, F7, F8
.local Base

; Id = .ident(Name)

.ifblank F1
  .error .sprintf("Must have at least one frame for metasprite %s", Name)
.endif

Base = .ident( .sprintf("METASPRITE_%d", METASPRITES_COUNT) )
.ident( .sprintf("METASPRITE_%d", METASPRITES_COUNT) ):
; setup the animation frame offset table
; Data1 = .ident( .sprintf("METASPRITE_%s_DATA_%d", Name, F1) )
; .byte Data1 - Mspr - 1
MetaspriteOffset Base, Name, F1, 1
.ifnblank F2
MetaspriteOffset Base, Name, F2, 2
.endif
.ifnblank F3
MetaspriteOffset Base, Name, F3, 3
.endif
.ifnblank F4
MetaspriteOffset Base, Name, F4, 4
.endif
.ifnblank F5
MetaspriteOffset Base, Name, F5, 5
.endif
.ifnblank F6
MetaspriteOffset Base, Name, F6, 6
.endif
.ifnblank F7
MetaspriteOffset Base, Name, F7, 7
.endif
.ifnblank F8
MetaspriteOffset Base, Name, F8, 8
.endif

IncludeMetasprite Name, F1

.ifnblank F2
IncludeMetasprite Name, F2
.endif
.ifnblank F3
IncludeMetasprite Name, F3
.endif
.ifnblank F4
IncludeMetasprite Name, F4
.endif
.ifnblank F5
IncludeMetasprite Name, F5
.endif
.ifnblank F6
IncludeMetasprite Name, F6
.endif
.ifnblank F7
IncludeMetasprite Name, F7
.endif
.ifnblank F8
IncludeMetasprite Name, F8
.endif

MetaspriteReserve .sprintf("%s_%s", Name, Animation)

.endmacro



.segment "_pnoinit"
shuffle_offset: .res 1


.struct OAM
  Ypos .byte
  Tile .byte
  Attr .byte
  Xpos .byte
.endstruct


.segment "_pprg__rom__2"


MetaspriteReserve "NULL"
METASPRITE_0 = $0000

MetaspriteAnimation "kitty", "walk_up",     0, 1, 2, 3
MetaspriteAnimation "kitty", "walk_right",  4, 5, 6, 7
MetaspriteAnimation "kitty", "walk_down",   8, 9, 10, 11
MetaspriteAnimation "kitty", "walk_left",   12, 13, 14, 15


MetaspriteAnimation "weapon_cube", "base",   0,  1,  2,  3
MetaspriteAnimation "weapon_cube", "atk1",   4,  5,  6,  7
MetaspriteAnimation "weapon_cube", "atk2",   8,  9,  10, 11
MetaspriteAnimation "weapon_cube", "atk3",   12, 13, 14, 15

MetaspriteAnimation "weapon_diamond", "base",   0,  1,  2,  3
MetaspriteAnimation "weapon_diamond", "atk1",   4,  5,  6,  7
MetaspriteAnimation "weapon_diamond", "atk2",   8,  9,  10, 11
MetaspriteAnimation "weapon_diamond", "atk3",   12, 13, 14, 15

MetaspriteAnimation "weapon_pyramid", "base",   0,  1,  2,  3
MetaspriteAnimation "weapon_pyramid", "atk1",   4,  5,  6,  7
MetaspriteAnimation "weapon_pyramid", "atk2",   8,  9,  10, 11
MetaspriteAnimation "weapon_pyramid", "atk3",   12, 13, 14, 15

MetaspriteAnimation "weapon_sphere", "base",   0,  1,  2,  3
MetaspriteAnimation "weapon_sphere", "atk1",   4,  5,  6,  7
MetaspriteAnimation "weapon_sphere", "atk2",   8,  9,  10, 11
MetaspriteAnimation "weapon_sphere", "atk3",   12, 13, 14, 15


.segment "_pprg__rom__2"





;;;;
; Internal use
; Generate the lookup tables for the metasprites used by the renderer.
; All metasprites must be defined before this part

MetaspriteTableLo:
.repeat METASPRITES_COUNT, I
  .byte .lobyte(.ident(.sprintf("METASPRITE_%d", I)))
.endrepeat
MetaspriteTableHi:
.repeat METASPRITES_COUNT, I
  .byte .hibyte(.ident(.sprintf("METASPRITE_%d", I)))
.endrepeat

ROOM_X_LO = 3
ROOM_X_HI = 4
ROOM_Y_LO = 5
ROOM_Y_HI = 6

; choose two numbers that are co-prime with the OBJECT_COUNT
OBJECT_SHUFFLE_OFFSET1 = 13
OBJECT_SHUFFLE_OFFSET2 = 9


SIZE_OF_OBJECT = 22
SIZE_OF_SOLID_OBJECT = 7
OBJECT_COUNT = 16
OBJECT_X_FRAC     = 0  * OBJECT_COUNT
OBJECT_X_LO       = 1  * OBJECT_COUNT
OBJECT_X_HI       = 2  * OBJECT_COUNT
OBJECT_Y_FRAC     = 3  * OBJECT_COUNT
OBJECT_Y_LO       = 4  * OBJECT_COUNT
OBJECT_Y_HI       = 5  * OBJECT_COUNT
OBJECT_HITBOX_X   = 6  * OBJECT_COUNT
OBJECT_HITBOX_Y   = 7  * OBJECT_COUNT
OBJECT_HITBOX_W   = 8  * OBJECT_COUNT
OBJECT_HITBOX_H   = 9  * OBJECT_COUNT
OBJECT_COLLISION  = 10 * OBJECT_COUNT
OBJECT_METASPRITE = 11 * OBJECT_COUNT
OBJECT_STATE      = 12 * OBJECT_COUNT
OBJECT_TILE_OFFSET= 13 * OBJECT_COUNT
OBJECT_ANIM_FRAME = 14 * OBJECT_COUNT
OBJECT_FRAME_COUNT= 15 * OBJECT_COUNT
OBJECT_ATTRIBUTE  = 16 * OBJECT_COUNT
OBJECT_DIRECTION  = 17 * OBJECT_COUNT
OBJECT_SPEED      = 18 * OBJECT_COUNT
OBJECT_HP         = 19 * OBJECT_COUNT


.import sprite_slot, objects, OAM_BUF, room, hud_tile_offset
.importzp global_timer, view_x, view_y

Ptr = $02 ; __rc2
Atr = $04 ; __rc4
Xlo = $05 ; __rc5
Xhi = $06 ; __rc6
Ylo = $07 ; __rc7
Yhi = $08 ; __rc8
LoopCount = $09 ; __rc9
ScreenLeftX = $0a
ScreenLeftY = $0c
Tile = $0e ; __rc14

DrawMetasprite:
  cpx #0
  beq @Exit
  cpy #1
  bne :+
    jsr .loword(DrawHud)
    ; Really ghetto, but just draw the HUD whenever slot 1 is drawn
    ldy #1
    ldx objects + OBJECT_METASPRITE,y
  :
  lda objects + OBJECT_STATE,y
  ; If state is negative then the object is hidden
  bpl :+
@Exit:
    rts
  :

  lda objects + OBJECT_X_LO,y
  sec
  sbc ScreenLeftX
  sta Xlo
  lda objects + OBJECT_X_HI,y
  sbc ScreenLeftX+1
  sta Xhi
  lda objects + OBJECT_Y_LO,y
  sec
  sbc ScreenLeftY
  sta Ylo
  lda objects + OBJECT_Y_HI,y
  sbc ScreenLeftY+1
  sta Yhi

  lda objects + OBJECT_ATTRIBUTE,y
  sta Atr

  lda objects + OBJECT_TILE_OFFSET,y
  sta Tile


  lda .loword(MetaspriteTableLo),x
  sta Ptr
  lda .loword(MetaspriteTableHi),x
  sta Ptr+1
  
  ; Check the animation data and frame
  ldx sprite_slot
  lda objects + OBJECT_ANIM_FRAME,y
  tay
  lda (Ptr),y
  tay
  bpl RenderLoop ; unconditional

; Offscreen sprites end up here

Skip4:   ; X Offscreen
    dey
Skip3:   ; Y Offscreen
    dey
Skip2:
    dey
    ; Move this sprite offscreen
    lda #$f8
    sta OAM_BUF + OAM::Ypos,x
    inx
    inx
    inx
    inx
    dey
    ; beq LoopEnded
RenderLoop:
    ; load the x position and make sure its on screen
    clc
    lda (Ptr),y
    cmp #$7f
    beq LoopEnded
    cmp #0
    bpl PositiveX
      adc Xlo
      sta OAM_BUF + OAM::Xpos,x
      lda Xhi
      adc #$ff
      beq ContinueAfterX
      bne Skip4
  PositiveX:
    adc Xlo
    sta OAM_BUF + OAM::Xpos,x
    lda Xhi
    adc #0
    bne Skip4
  ContinueAfterX:
    dey

    ; load the y position and also make sure its on screen
    clc
    lda (Ptr),y
    bpl PositiveY
      ; NegativeY
      adc Ylo
      sta OAM_BUF + OAM::Ypos,x
      lda Yhi
      adc #$ff
      ; cmp #1      ; page 1 is the "main" y position
      beq ContinueAfterY
      bne Skip3
  PositiveY:
    adc Ylo
    sta OAM_BUF + OAM::Ypos,x
    lda Yhi
    adc #0
    ; cmp #1      ; page 1 is the "main" y position
    bne Skip3
  ContinueAfterY:
    dey

    ; Mix attributes but if the NO_PALETTE bit is set, prevent
    ; the palette from changing.
    lda (Ptr),y
;     bit NoPaletteBitMask
;     beq AllowPaletteChange
;       ; No palette change bit set, so pull the byte and
;       ; mask off the palette from the attribute byte
;       lda Atr
;       and #%11111100
;       ora (Ptr),y
;       bne WritePalette ; unconditional
; AllowPaletteChange:
    ora Atr
; WritePalette:
    sta OAM_BUF + OAM::Attr,x
    dey

    ; set the tile number and move to the next sprite
    lda (Ptr),y
    clc
    adc Tile
    sta OAM_BUF + OAM::Tile,x
    inx
    inx
    inx
    inx
    dey
    bne RenderLoop
LoopEnded:
  stx sprite_slot
  rts

; NoPaletteBitMask:
;   .byte (SPR_NO_PALETTE >> 8)


.export draw_all_metasprites := DrawAllMetasprites
DrawAllMetasprites:

  ; DEBUG - clear metasprites
  ; jsr .loword(MoveAllSpritesOffscreen)

  lda room + ROOM_X_LO
  clc
  adc view_x
  sta ScreenLeftX
  lda room + ROOM_X_HI
  adc #0
  sta ScreenLeftX+1
  lda room + ROOM_Y_LO
  clc
  adc view_y
  sta ScreenLeftY
  lda room + ROOM_Y_HI
  adc #0
  sta ScreenLeftY+1

  ; Draw player / weapon first
  lda #0
  sta sprite_slot
  ldx objects + OBJECT_METASPRITE
  beq PlayerDrawn
  cpx #METASPRITE_kitty_walk_down
  ; If we are walking downwards, draw the orb first to make it look like its above the player
  beq SwitchedOrder
    tay
    jsr .loword(DrawMetasprite)
    ldy #1
    ldx objects + OBJECT_METASPRITE,y
    jsr .loword(DrawMetasprite)
    jmp .loword(PlayerDrawn)
SwitchedOrder:
    ldy #1
    ldx objects + OBJECT_METASPRITE,y
    jsr .loword(DrawMetasprite)
    ldy #0
    ldx objects + OBJECT_METASPRITE,y
    jsr .loword(DrawMetasprite)
PlayerDrawn:

  lda #OBJECT_COUNT - 1 ; size of the different object update list
  sta LoopCount
  lda shuffle_offset
  clc
  adc #OBJECT_SHUFFLE_OFFSET1
  cmp #OBJECT_COUNT
  bcc :+
    ; implicit carry set
    sbc #OBJECT_COUNT
:
  sta shuffle_offset
  lda FRAME_CNT1
  lsr
  lda shuffle_offset
  bcc ObjectLoopNegative

ObjectLoopPositive:
    clc
    adc #OBJECT_SHUFFLE_OFFSET2
    cmp #OBJECT_COUNT
    bcc :+
      ; implicit carry set
      sbc #OBJECT_COUNT
    :
    ; skip index zero since we draw the player first always.
    cmp #2
    bcc NextLoop
      ; TODO check offscreenbits to make sure they are onscreen still
      tay
      ldx objects + OBJECT_METASPRITE,y
      ; Check for null metasprite
      beq NextLoop
      ; cpx #METASPRITES_COUNT ; todo remove this after fixing all bugs
      ; bcs NextLoop
        sta shuffle_offset
        jsr .loword(DrawMetasprite)
        lda shuffle_offset
  NextLoop:
    dec LoopCount
    bpl ObjectLoopPositive
  jmp .loword(FinishedDrawing)

ObjectLoopNegative:
    sec
    sbc #OBJECT_SHUFFLE_OFFSET2
    bcs :+
      ; implicit carry clear
      adc #OBJECT_COUNT
    :
    ; skip index zero since we draw the player first always.
    cmp #2
    bcc NextLoop2
      ; TODO check offscreenbits to make sure they are onscreen still
      tay
      ldx objects + OBJECT_METASPRITE,y
      ; Check for null metasprite
      beq NextLoop2
      ; cpx #METASPRITES_COUNT ; todo remove this after fixing all bugs
      ; bcs NextLoop2
        sta shuffle_offset
        jsr .loword(DrawMetasprite)
        lda shuffle_offset
  NextLoop2:
    dec LoopCount
    bpl ObjectLoopNegative

FinishedDrawing:
  sta shuffle_offset

  ; Clear sprites up to the offset
  ; Calculate sprite_slot / 4 * 3 and add that to the OAM clear to jump
  ; the middle of the clear routine based on how many we need to clear
  lda sprite_slot
  lsr 
  sta Ptr
  lsr 
  ; clc - always cleared
  adc Ptr
  ; clc - always cleared
  adc #<OAMClear
  sta Ptr
  lda #>OAMClear
  adc #0
  sta Ptr+1
  lda #$f8
  jmp (Ptr)

DrawHud:

CAT_L  = 4
CAT_R  = 8
HUD_HP = 0

  ldx sprite_slot
  lda hud_tile_offset
  clc
  adc #10 * 2
  sta OAM_BUF + OAM::Tile + CAT_L, x
  ; carry clear
  adc #2
  sta OAM_BUF + OAM::Tile + CAT_R, x
  lda objects + OBJECT_HP
  asl
  clc
  adc hud_tile_offset
  sta OAM_BUF + OAM::Tile + HUD_HP, x

  lda #0
  sta OAM_BUF + OAM::Attr + CAT_L, x
  sta OAM_BUF + OAM::Attr + CAT_R, x
  sta OAM_BUF + OAM::Attr + HUD_HP, x

  lda #24
  sta OAM_BUF + OAM::Ypos + CAT_L, x
  sta OAM_BUF + OAM::Ypos + CAT_R, x
  sta OAM_BUF + OAM::Ypos + HUD_HP, x

  lda #20
  sta OAM_BUF + OAM::Xpos + CAT_L, x
  lda #28
  sta OAM_BUF + OAM::Xpos + CAT_R, x
  lda #40
  sta OAM_BUF + OAM::Xpos + HUD_HP, x

  txa
  clc
  adc #12
  tax
  sta sprite_slot

  rts

.export move_all_sprites_offscreen := MoveAllSpritesOffscreen
MoveAllSpritesOffscreen:
  ldy #0
  sty sprite_slot
  lda #$f8
OAMClear:
.repeat 64, I
    sta OAM_BUF + OAM::Ypos + I*4 ; write 248 into OAM data's Y coordinate
.endrepeat
  rts


