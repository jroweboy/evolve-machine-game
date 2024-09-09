
.include "nes.inc"

A53_REG_SELECT	= $5000
A53_REG_VALUE	= $8000

huffmunch_zpblock = $02
.globl huffmunch_zpblock

decompress_buffer = $100
.globl decompress_buffer

.section .prg_rom_1.compress,"ax",@progbits
.globl donut_decompress_vram
donut_decompress_vram:
    asl
    tax
    ; clc always clear if a < 128
    lda compressed_data,x
    adc #<compressed_data
    sta __rc2
    lda compressed_data+1,x
    adc #>compressed_data
    sta __rc3
    jmp donut_block

.globl donut_decompress_buffer
donut_decompress_buffer:
    asl
    tax
    ; clc always clear if a < 128
    lda compressed_data,x
    adc #<compressed_data
    sta __rc2
    lda compressed_data+1,x
    adc #>compressed_data
    sta __rc3
    ldx #0
    jmp donut_decompress_block

; huffmunch code
;     tax
;     lda #<compressed_data
;     sta huffmunch_zpblock+0
;     lda #>compressed_data
;     sta huffmunch_zpblock+1
;     ldy #0
;     jsr huffmunch_load
;     sty __rc1
;     stx __rc0
;     cpx #0
;     bne .A1
;         ; Special case for if the low byte is $00 but the high byte is not
;         jsr huffmunch_read
;         sta $2007
;         dec __rc0
;         dec __rc1
;         bmi .A2
; .A1:
;         jsr huffmunch_read
;         sta $2007
;         dec __rc0
;         bne .A1
;     dec __rc1
;     bpl .A1
; .A2:
;     rts



; .globl donut_decompress_buffer
; donut_decompress_buffer:
;     tax
;     lda #<compressed_data
;     sta huffmunch_zpblock+0
;     lda #>compressed_data
;     sta huffmunch_zpblock+1
;     ldy #0
;     sty __rc1
;     jsr huffmunch_load
;     stx __rc0
;     ; cpx #0
;     ; beq .B2
; .B1:
;         jsr huffmunch_read
;         ldy __rc1
;         sta decompress_buffer,y
;         inc __rc1
;         dec __rc0
;         bne .B1
; .B2:
;     rts

.section .prg_rom_1.compressed_data,"aR",@progbits
compressed_data:
.incbin "compressed/archive.dnt"

; .globl donut_decompress
; donut_decompress:
;     ; __rc2 and __rc3 is the pointer (and its setup in ca65 to use these values)
;     lda #0
;     sta __rc4
;     ; ldx #0
;     jmp donut_block

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
    sta sfx_queue1
    sta sfx_queue2

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
