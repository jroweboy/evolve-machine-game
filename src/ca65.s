
.include "nes.inc"

A53_REG_SELECT	= $5000
A53_REG_VALUE	= $8000

.section .text.donut,"ax",@progbits
.globl donut_decompress
donut_decompress:
    ; __rc2 and __rc3 is the pointer (and its setup in ca65 to use these values)
    lda #0
    sta __rc4
    ; ldx #0
    jmp donut_block

; set this to run after the ram clearing but before the find ppu frame wait
.section .init.210,"axR",@progbits
audio_init:
    ; disable the APU framecounter IRQ source and set next_song to init
    lda #$ff
    sta $4017
    sta next_song
    
    jsr __run_audio
    lda #$fe
    sta next_song
    sta current_song

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
    jmp music_nmi_callback

.section .nmi.200,"axR",@progbits
inc_global_timer:
    inc global_timer
    bne .L1
        inc global_timer+1
        bne .L1
            inc global_timer+2
.L1:

.section .nmi.055,"axR",@progbits
.globl oam_update_nmi
oam_update_nmi:
    lda #>OAM_BUF
    sta OAMDMA
