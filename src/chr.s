
.section .prg_rom_1,"a"
.globl title_screen_bin
title_screen_bin:
    .incbin "chr/titlescreen.bin"

.section .prg_rom_1,"a"
.globl test_map_bin
test_map_bin:
    .incbin "chr/test_map.bin"
