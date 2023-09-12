
.section .prg_rom_1,"a"
.globl title_chr
title_chr:
    .incbin "graphics/titlescreen_chr.bin"
.byte $ff, $ff

.section .prg_rom_1,"a"
.globl room_chr
room_chr:
    .incbin "graphics/room_chr.bin"
.byte $ff, $ff

.section .prg_rom_1,"a"
.globl title_bin
title_bin:
    .incbin "graphics/titlescreen.bin"
.byte $ff, $ff


.section .prg_rom_1,"a"
.globl bottom_bin
bottom_bin:
    .incbin "graphics/bottom.bin"
.byte $ff, $ff


.section .prg_rom_1,"a"
.globl left_bin
left_bin:
    .incbin "graphics/left.bin"
.byte $ff, $ff


.section .prg_rom_1,"a"
.globl right_bin
right_bin:
    .incbin "graphics/right.bin"
.byte $ff, $ff


.section .prg_rom_1,"a"
.globl single_bin
single_bin:
    .incbin "graphics/single.bin"
.byte $ff, $ff


.section .prg_rom_1,"a"
.globl start_bin
start_bin:
    .incbin "graphics/start.bin"
.byte $ff, $ff


.section .prg_rom_1,"a"
.globl top_bin
top_bin:
    .incbin "graphics/top.bin"
.byte $ff, $ff
