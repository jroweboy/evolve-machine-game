


FAMISTUDIO_CFG_EXTERNAL = 1
FAMISTUDIO_EXP_EPSM = 1
FAMISTUDIO_USE_RELEASE_NOTES = 1
FAMISTUDIO_USE_PITCH_TRACK = 1
FAMISTUDIO_USE_SLIDE_NOTES = 1
FAMISTUDIO_USE_NOISE_SLIDE_NOTES = 1
FAMISTUDIO_CFG_DPCM_SUPPORT = 1
FAMISTUDIO_USE_DELTA_COUNTER = 1
FAMISTUDIO_DPCM_OFF = $c000

.define FAMISTUDIO_CA65_ZP_SEGMENT ZP
.define FAMISTUDIO_CA65_RAM_SEGMENT RAM
.define FAMISTUDIO_CA65_CODE_SEGMENT BANKED

.segment "RAM"

current_song: .res 1
next_song:    .res 1

.segment "BANKED"

nmi_callback:
    lda next_song
    bpl check_for_song_change
    ; special operation. check for init flag
    cmp #$ff
    beq init_famistudio
    ; TODO add silence tracks and whatever
    ; nothing to do until we add that
    rts
init_famistudio:
    ; if the next song is $ff, that means we need to init famistudio
    lda #1
    ldx #<music_data
    ldy #>music_data
    jmp famistudio_init
    ; rts
check_for_song_change:
    cmp current_song
    beq continue_playing_song
        ; song changed so switch to the new track
        sta current_song
        jsr famistudio_music_play 
continue_playing_song:
    ; finally update famistudio
    jmp famistudio_update
    ; rts


.include "famistudio_ca65.s"

; include the actual song data in the same bank as the driver now.
music_data:
.include "evolve_machine.s"
