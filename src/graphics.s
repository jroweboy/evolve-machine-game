
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
    .incbin "graphics/chr/start.chr.dnt"

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
.globl start_bin
start_bin:
    .incbin "graphics/nmt/start.nmt.dnt"

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
.globl start_attr
start_attr:
    .incbin "graphics/atr/start.atr"
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
    .incbin "graphics/pal/start.pal"
.section .prg_rom_1,"aR",@progbits
.globl updown_palette
updown_palette:
    .incbin "graphics/pal/leftright.pal"
