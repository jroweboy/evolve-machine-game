
.include "header/graphics_constants.s"
;;;; Section LUT

.set MIRROR_LOWER_BANK, 0
.set MIRROR_UPPER_BANK, 1
.set MIRROR_VERTICAL, 2
.set MIRROR_HORIZONTAL, 3

.section .prg_rom_1.section_lut,"aR",@progbits
.globl section_lut
section_lut:
    ; nametable
    .byte bottom_bin@mos16lo, left_bin@mos16lo, right_bin@mos16lo, single_bin@mos16lo, startdown_bin@mos16lo, startup_bin@mos16lo, top_bin@mos16lo
    .byte bottom_bin@mos16hi, left_bin@mos16hi, right_bin@mos16hi, single_bin@mos16hi, startdown_bin@mos16hi, startup_bin@mos16hi, top_bin@mos16hi
    ; chr
    .byte room_updown_chr@mos16lo, room_leftright_chr@mos16lo, room_leftright_chr@mos16lo, room_single_chr@mos16lo, room_start_chr@mos16lo, room_start_chr@mos16lo, room_updown_chr@mos16lo
    .byte room_updown_chr@mos16hi, room_leftright_chr@mos16hi, room_leftright_chr@mos16hi, room_single_chr@mos16hi, room_start_chr@mos16hi, room_start_chr@mos16hi, room_updown_chr@mos16hi
    ; attribute
    .byte bottom_attr@mos16lo, left_attr@mos16lo, right_attr@mos16lo, single_attr@mos16lo, startdown_attr@mos16lo, startup_attr@mos16lo, top_attr@mos16lo
    .byte bottom_attr@mos16hi, left_attr@mos16hi, right_attr@mos16hi, single_attr@mos16hi, startdown_attr@mos16hi, startup_attr@mos16hi, top_attr@mos16hi
    ; palette ? do we need this?
    .byte updown_palette@mos16lo, leftright_palette@mos16lo, leftright_palette@mos16lo, single_palette@mos16lo, start_palette@mos16lo, start_palette@mos16lo, updown_palette@mos16lo
    .byte updown_palette@mos16hi, leftright_palette@mos16hi, leftright_palette@mos16hi, single_palette@mos16hi, start_palette@mos16hi, start_palette@mos16hi, updown_palette@mos16hi
    ; mirroring
    .byte MIRROR_HORIZONTAL, MIRROR_VERTICAL, MIRROR_VERTICAL, MIRROR_VERTICAL, MIRROR_HORIZONTAL, MIRROR_HORIZONTAL, MIRROR_HORIZONTAL
    ; chr offset
    .byte updown_chr_offset@mos16lo, leftright_chr_offset@mos16lo, leftright_chr_offset@mos16lo, single_chr_offset@mos16lo, startupstartdown_chr_offset@mos16lo, startupstartdown_chr_offset@mos16lo, updown_chr_offset@mos16lo
    .byte updown_chr_offset@mos16hi, leftright_chr_offset@mos16hi, leftright_chr_offset@mos16hi, single_chr_offset@mos16hi, startupstartdown_chr_offset@mos16hi, startupstartdown_chr_offset@mos16hi, updown_chr_offset@mos16hi
    ; chr count
    .byte updown_chr_count, leftright_chr_count, leftright_chr_count, single_chr_count, startupstartdown_chr_count, startupstartdown_chr_count, updown_chr_count


;;;; CHR (Compressed)

.section .prg_rom_1,"aR",@progbits
.globl title_chr
title_chr:
    .incbin "graphics/chr/titlescreen.chr.dnt"
.section .prg_rom_1,"aR",@progbits
.globl room_updown_chr
room_updown_chr:
    .incbin "graphics/chr/updown.chr.dnt"
.globl room_leftright_chr
room_leftright_chr:
    .incbin "graphics/chr/leftright.chr.dnt"
.globl room_single_chr
room_single_chr:
    .incbin "graphics/chr/single.chr.dnt"
.globl room_start_chr
room_start_chr:
    .incbin "graphics/chr/startupstartdown.chr.dnt"
.globl hudfont_chr
hudfont_chr:
    .incbin "graphics/chr/hudfont.chr.dnt"

;;;; Nametables (Compressed)

.section .prg_rom_1,"aR",@progbits
.globl title_bin
title_bin:
    .incbin "graphics/nmt/titlescreen_atr.nmt.dnt"

.section .prg_rom_1,"aR",@progbits
.globl bottom_bin
bottom_bin:
    .incbin "graphics/nmt/down.nmt.dnt"

.section .prg_rom_1,"aR",@progbits
.globl left_bin
left_bin:
    .incbin "graphics/nmt/left.nmt.dnt"

.section .prg_rom_1,"aR",@progbits
.globl right_bin
right_bin:
    .incbin "graphics/nmt/right.nmt.dnt"

.section .prg_rom_1,"aR",@progbits
.globl single_bin
single_bin:
    .incbin "graphics/nmt/single.nmt.dnt"

.section .prg_rom_1,"aR",@progbits
.globl startup_bin
startup_bin:
    .incbin "graphics/nmt/startup.nmt.dnt"

.section .prg_rom_1,"aR",@progbits
.globl startdown_bin
startdown_bin:
    .incbin "graphics/nmt/startdown.nmt.dnt"

.section .prg_rom_1,"aR",@progbits
.globl top_bin
top_bin:
    .incbin "graphics/nmt/up.nmt.dnt"

;;;;;; Attributes

.section .prg_rom_1,"aR",@progbits
.globl bottom_attr
bottom_attr:
    .incbin "graphics/atr/down.atr"
.section .prg_rom_1,"aR",@progbits
.globl left_attr
left_attr:
    .incbin "graphics/atr/left.atr"
.section .prg_rom_1,"aR",@progbits
.globl right_attr
right_attr:
    .incbin "graphics/atr/right.atr"
.section .prg_rom_1,"aR",@progbits
.globl single_attr
single_attr:
    .incbin "graphics/atr/single.atr"
.section .prg_rom_1,"aR",@progbits
.globl startup_attr
startup_attr:
    .incbin "graphics/atr/startup.atr"
.section .prg_rom_1,"aR",@progbits
.globl startdown_attr
startdown_attr:
    .incbin "graphics/atr/startdown.atr"
.section .prg_rom_1,"aR",@progbits
.globl top_attr
top_attr:
    .incbin "graphics/atr/up.atr"

;;;;;; Palettes

.section .prg_rom_1,"aR",@progbits
.globl title_palette
title_palette:
    .incbin "graphics/pal/titlescreen.pal"
.section .prg_rom_1,"aR",@progbits
.globl leftright_palette
leftright_palette:
    .incbin "graphics/pal/leftright.pal"
.section .prg_rom_1,"aR",@progbits
.globl single_palette
single_palette:
    .incbin "graphics/pal/single.pal"
.section .prg_rom_1,"aR",@progbits
.globl start_palette
start_palette:
    .incbin "graphics/pal/startupstartdown.pal"
.section .prg_rom_1,"aR",@progbits
.globl updown_palette
updown_palette:
    .incbin "graphics/pal/leftright.pal"

;;;;;;;; Objects

.section .prg_rom_1,"aR",@progbits
.globl door_down_chr_dnt
door_down_chr_dnt:
    .incbin "graphics/chr/door_down.chr.dnt"
.globl door_up_chr_dnt
door_up_chr_dnt:
    .incbin "graphics/chr/door_up.chr.dnt"
.globl door_left_chr_dnt
door_left_chr_dnt:
    .incbin "graphics/chr/door_left.chr.dnt"
.globl door_right_chr_dnt
door_right_chr_dnt:
    .incbin "graphics/chr/door_right.chr.dnt"
.globl door_exit_chr_lut
door_exit_chr_lut:
    .word door_up_chr_dnt, door_right_chr_dnt, door_down_chr_dnt, door_left_chr_dnt
