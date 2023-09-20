
.section .prg_rom_1,"aR",@progbits
.globl title_chr
title_chr:
    .incbin "graphics/titlescreen_chr.bin"
.byte $ff, $ff

.section .prg_rom_1,"aR",@progbits
.globl room_chr
room_chr:
    .incbin "graphics/room_chr.bin"
.byte $ff, $ff

.section .prg_rom_1,"aR",@progbits
.globl title_bin
title_bin:
    .incbin "graphics/titlescreen.bin"
.byte $ff, $ff


.section .prg_rom_1,"aR",@progbits
.globl bottom_bin
bottom_bin:
    .incbin "graphics/bottom.bin"
.byte $ff, $ff


.section .prg_rom_1,"aR",@progbits
.globl left_bin
left_bin:
    .incbin "graphics/left.bin"
.byte $ff, $ff


.section .prg_rom_1,"aR",@progbits
.globl right_bin
right_bin:
    .incbin "graphics/right.bin"
.byte $ff, $ff


.section .prg_rom_1,"aR",@progbits
.globl single_bin
single_bin:
    .incbin "graphics/single.bin"
.byte $ff, $ff


.section .prg_rom_1,"aR",@progbits
.globl start_bin
start_bin:
    .incbin "graphics/start.bin"
.byte $ff, $ff


.section .prg_rom_1,"aR",@progbits
.globl top_bin
top_bin:
    .incbin "graphics/top.bin"
.byte $ff, $ff
