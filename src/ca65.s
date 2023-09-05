
.globl _current_music
_current_music = $300
.globl _next_music
_next_music = $301

A53_REG_SELECT	= $5000
A53_REG_VALUE	= $8000

.section .init.100,"axR",@progbits
	lda #$80
	sta A53_REG_SELECT
    lda #0b00011100
	sta A53_REG_VALUE
    lda #$ff
    sta _next_music
	lda #1
	sta A53_REG_SELECT
    lda #0
	sta A53_REG_VALUE
	jsr __run_audio

.section .nmi.100,"axR",@progbits
	jsr __run_audio

.section .text.audio,"ax",@progbits
.weak audio
.globl __run_audio
__run_audio:
    jmp custom_audio

.section .prg_rom_0,"a"
.globl custom_audio
custom_audio:
    .incbin "ca65/prg8.bin"

.section .prg_rom_fixed,"a"
.globl donut_decompress_block
donut_decompress_block:
    .incbin "ca65/prgc.bin"
