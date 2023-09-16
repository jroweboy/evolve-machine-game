

A53_REG_SELECT	= $5000
A53_REG_VALUE	= $8000

; Reserve 0x110 bytes of data?
.section .noinit.ca65_reserved,"aw",@nobits
.globl ca65_reserved
ca65_reserved:
  .zero 0x110

.globl _current_music
_current_music = ca65_reserved

.globl _next_music
_next_music = ca65_reserved + 1

.globl _sfx_queue
_sfx_queue = ca65_reserved + 2

; This is a bit hacky, but we put donut compression at $c003
; by marking this as the highest priority init block
; and just jump over this block during init
.section .init.000,"axR",@progbits
    jmp after_donut_block
.globl donut_decompress_block
donut_decompress_block:
    .incbin "ca65/prgc.bin"
after_donut_block:


.section .text.donut,"ax",@progbits
.globl donut_decompress
donut_decompress:
    lda __rc2
    sta $ed
    lda __rc3
    sta $ee
    lda #0
    sta $ef
    jmp donut_decompress_block

; set this to run after the ram clearing but before the find ppu frame wait
.section .init.210,"axR",@progbits
audio_init:
    ; disable the APU framecounter IRQ source and set _next_music to init
    lda #$ff
    sta $4017
    sta _next_music
    jsr __run_audio
    lda #$fe
    sta _next_music
    sta _current_music

.section .nmi.200,"axR",@progbits
    lda #2
    sta $4123
    jsr __run_audio
    lda #0
    sta $4123

.section .text.audio,"ax",@progbits
.globl __run_audio
__run_audio:
    lda #1
    sta A53_REG_SELECT
    lda #0
    sta A53_REG_VALUE
    jmp custom_audio

.section .nmi.200,"axR",@progbits
inc_global_timer:
    inc global_timer
    bne .L1
        inc global_timer+1
        bne .L1
            inc global_timer+2
.L1:

.section .text.audio,"ax",@progbits
.globl play_song
play_song:
    sta _next_music
    rts

.section .text.audio,"ax",@progbits
.globl play_sfx
play_sfx:
    sta _sfx_queue
    rts

; We don't have to do any crazy shenanigans to for famistudio to the start of the bank
; because its the only thing in this bank.
.section .prg_rom_0,"a"
.globl custom_audio
custom_audio:
    .incbin "ca65/prg8.bin"
