
FAMISTUDIO_CFG_EXTERNAL = 1
FAMISTUDIO_EXP_EPSM = 1
FAMISTUDIO_USE_RELEASE_NOTES = 1
FAMISTUDIO_USE_PITCH_TRACK = 1
FAMISTUDIO_USE_SLIDE_NOTES = 1
FAMISTUDIO_USE_NOISE_SLIDE_NOTES = 1
FAMISTUDIO_USE_DELTA_COUNTER = 1
FAMISTUDIO_CFG_DPCM_SUPPORT = 1
FAMISTUDIO_EXP_EPSM_SSG_CHN_CNT        = 0
FAMISTUDIO_EXP_EPSM_FM_CHN_CNT         = 3
FAMISTUDIO_EXP_EPSM_RHYTHM_CHN1_ENABLE = 0
FAMISTUDIO_EXP_EPSM_RHYTHM_CHN2_ENABLE = 0
FAMISTUDIO_EXP_EPSM_RHYTHM_CHN3_ENABLE = 0
FAMISTUDIO_EXP_EPSM_RHYTHM_CHN4_ENABLE = 0
FAMISTUDIO_EXP_EPSM_RHYTHM_CHN5_ENABLE = 0
FAMISTUDIO_EXP_EPSM_RHYTHM_CHN6_ENABLE = 0

FAMISTUDIO_CFG_SFX_SUPPORT = 1
FAMISTUDIO_CFG_SFX_STREAMS = 2
FAMISTUDIO_DPCM_OFF = evolve_machine_dpcm

.define FAMISTUDIO_CA65_ZP_SEGMENT _pzp
.define FAMISTUDIO_CA65_RAM_SEGMENT _pnoinit
.define FAMISTUDIO_CA65_CODE_SEGMENT _pprg__rom__0

.segment "_pnoinit"

.export current_song, next_song, sfx_queue1, sfx_queue2

current_song: .res 1
next_song:    .res 1
sfx_queue1:   .res 1
sfx_queue2:   .res 1

.segment "_pprg__rom__0"

Silence = $fd
StopMusic = $fe
InitEngine = $ff

.export music_nmi_callback

music_nmi_callback:
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
        jsr famistudio_init
        ldx #<evolve_machine_sfx
        ldy #>evolve_machine_sfx
        jmp famistudio_sfx_init
    
    ; rts
check_for_song_change:
    cmp current_song
    beq continue_playing_song
        ; song changed so switch to the new track
        sta current_song
        jsr famistudio_music_play
continue_playing_song:
    lda sfx_queue1
    bmi :+
        ldx #FAMISTUDIO_SFX_CH0
        jsr famistudio_sfx_play
    :
    lda sfx_queue2
    bmi :+
        ldx #FAMISTUDIO_SFX_CH1
        jsr famistudio_sfx_play
    :
    lda #$ff
    sta sfx_queue1
    sta sfx_queue2
    ; finally update famistudio
    jmp famistudio_update
    ; rts

.include "famistudio_ca65.s"

; include the actual song data in the same bank as the driver now.
music_data:
.include "evolve_machine.s"

.segment "_pdpcm"
evolve_machine_dpcm:
.incbin "evolve_machine.dmc"

.segment "_pprg__rom__fixed"
evolve_machine_sfx:
.include "evolve_machine_sfx.s"

