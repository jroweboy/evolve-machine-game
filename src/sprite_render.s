
.section .prg_rom_1,"aR",@progbits
.globl kitty_chr
kitty_chr:
.incbin "sprites/kitty.chr.dnt"
.byte $ff, $ff

.section .prg_rom_2,"aR",@progbits
.globl kitty_walk_up
kitty_walk_up:
.byte 1f-.-1 + 0
.byte 2f-.-1 + 1
.byte 3f-.-1 + 2
.byte 4f-.-1 + 3
0:
.incbin "sprites/kitty_metasprite_0.bin"
1:
.incbin "sprites/kitty_metasprite_1.bin"
2:
.incbin "sprites/kitty_metasprite_2.bin"
3:
.incbin "sprites/kitty_metasprite_3.bin"
4:

.section .prg_rom_2,"aR",@progbits
.globl kitty_walk_right
kitty_walk_right:
.byte 1f-.-1 + 0
.byte 2f-.-1 + 1
.byte 3f-.-1 + 2
.byte 4f-.-1 + 3
0:
.incbin "sprites/kitty_metasprite_4.bin"
1:
.incbin "sprites/kitty_metasprite_5.bin"
2:
.incbin "sprites/kitty_metasprite_6.bin"
3:
.incbin "sprites/kitty_metasprite_7.bin"
4:

.section .prg_rom_2,"aR",@progbits
.globl kitty_walk_down
kitty_walk_down:
.byte 1f-.-1 + 0
.byte 2f-.-1 + 1
.byte 3f-.-1 + 2
.byte 4f-.-1 + 3
0:
.incbin "sprites/kitty_metasprite_8.bin"
1:
.incbin "sprites/kitty_metasprite_9.bin"
2:
.incbin "sprites/kitty_metasprite_10.bin"
3:
.incbin "sprites/kitty_metasprite_11.bin"
4:

.section .prg_rom_2,"aR",@progbits
.globl kitty_walk_left
kitty_walk_left:
.byte 1f-.-1 + 0
.byte 2f-.-1 + 1
.byte 3f-.-1 + 2
.byte 4f-.-1 + 3
0:
.incbin "sprites/kitty_metasprite_12.bin"
1:
.incbin "sprites/kitty_metasprite_13.bin"
2:
.incbin "sprites/kitty_metasprite_14.bin"
3:
.incbin "sprites/kitty_metasprite_15.bin"
4:
