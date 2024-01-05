
FAMISTUDIO_CFG_EXTERNAL = 1
FAMISTUDIO_EXP_EPSM = 1
FAMISTUDIO_USE_RELEASE_NOTES = 1
FAMISTUDIO_USE_PITCH_TRACK = 1
FAMISTUDIO_USE_SLIDE_NOTES = 1
FAMISTUDIO_USE_NOISE_SLIDE_NOTES = 1
FAMISTUDIO_USE_DELTA_COUNTER = 1
FAMISTUDIO_CFG_DPCM_SUPPORT = 1
FAMISTUDIO_DPCM_OFF = evolve_machine_dpcm

.define FAMISTUDIO_CA65_ZP_SEGMENT _pzp
.define FAMISTUDIO_CA65_RAM_SEGMENT _pnoinit
.define FAMISTUDIO_CA65_CODE_SEGMENT _pprg__rom__0__extended

.segment "_pnoinit"

current_song: .res 1
next_song:    .res 1
sfx_queue:    .res 1

.segment "_pprg__rom__0__extended"

Silence = $fd
StopMusic = $fe
InitEngine = $ff

nmi_callback:
    lda next_song
    bpl check_for_song_change
    ; special operation. check for init flag
    cmp #InitEngine
    beq init_famistudio
    cmp #StopMusic
    bne continue_playing_song
        jsr famistudio_music_stop
        dec next_song
        jmp continue_playing_song

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

.segment "_pprg__rom__fixed"
evolve_machine_dpcm:
.incbin "evolve_machine.dmc"
