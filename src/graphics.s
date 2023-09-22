
.section .prg_rom_1,"aR",@progbits
.globl title_chr
title_chr:
    .incbin "graphics/chr/titlescreen_chr.bin"
.byte $ff, $ff

;;;; CHR (Compressed)

.section .prg_rom_1,"aR",@progbits
.globl room_updown_chr
room_updown_chr:
    .incbin "graphics/chr/room_updown_chr.bin"
.byte $ff, $ff
.globl room_leftright_chr
room_leftright_chr:
    .incbin "graphics/chr/room_leftright_chr.bin"
.byte $ff, $ff
.globl room_single_chr
room_single_chr:
    .incbin "graphics/chr/room_single_chr.bin"
.byte $ff, $ff
.globl room_start_chr
room_start_chr:
    .incbin "graphics/chr/room_start_chr.bin"
.byte $ff, $ff

;;;; Nametables (Compressed)

.section .prg_rom_1,"aR",@progbits
.globl title_bin
title_bin:
    .incbin "graphics/nmt/titlescreen.bin"
.byte $ff, $ff

.section .prg_rom_1,"aR",@progbits
.globl bottom_bin
bottom_bin:
    .incbin "graphics/nmt/bottom.bin"
.byte $ff, $ff

.section .prg_rom_1,"aR",@progbits
.globl left_bin
left_bin:
    .incbin "graphics/nmt/left.bin"
.byte $ff, $ff

.section .prg_rom_1,"aR",@progbits
.globl right_bin
right_bin:
    .incbin "graphics/nmt/right.bin"
.byte $ff, $ff

.section .prg_rom_1,"aR",@progbits
.globl single_bin
single_bin:
    .incbin "graphics/nmt/single.bin"
.byte $ff, $ff

.section .prg_rom_1,"aR",@progbits
.globl start_bin
start_bin:
    .incbin "graphics/nmt/start.bin"
.byte $ff, $ff

.section .prg_rom_1,"aR",@progbits
.globl top_bin
top_bin:
    .incbin "graphics/nmt/top.bin"
.byte $ff, $ff

;;;;;; Attributes

.section .prg_rom_1,"aR",@progbits
.globl bottom_attr
bottom_attr:
    .incbin "graphics/atr/bottom.attr"
.section .prg_rom_1,"aR",@progbits
.globl left_attr
left_attr:
    .incbin "graphics/atr/left.attr"
.section .prg_rom_1,"aR",@progbits
.globl right_attr
right_attr:
    .incbin "graphics/atr/right.attr"
.section .prg_rom_1,"aR",@progbits
.globl single_attr
single_attr:
    .incbin "graphics/atr/single.attr"
.section .prg_rom_1,"aR",@progbits
.globl start_attr
start_attr:
    .incbin "graphics/atr/start.attr"
.section .prg_rom_1,"aR",@progbits
.globl top_attr
top_attr:
    .incbin "graphics/atr/top.attr"

;;;;;; Palettes

.section .prg_rom_1,"aR",@progbits
.globl title_palette
title_palette:
    .incbin "graphics/pal/title_pal.bin"

.section .prg_rom_1,"aR",@progbits
.globl leftright_palette
leftright_palette:
    .incbin "graphics/pal/leftright_pal.bin"
.section .prg_rom_1,"aR",@progbits
.globl single_palette
single_palette:
    .incbin "graphics/pal/single_pal.bin"
.section .prg_rom_1,"aR",@progbits
.globl start_palette
start_palette:
    .incbin "graphics/pal/start_pal.bin"
.section .prg_rom_1,"aR",@progbits
.globl updown_palette
updown_palette:
    .incbin "graphics/pal/updown_pal.bin"
