.macro sound_data sound
    .byte <(sound >> 21)
    .byte <((sound >> 14) & $7f)
    .byte <((sound >> 6) & $ff)
.endmacro

; For VT32 systems: To add samples to this game, use sound_data macro
.segment "SOUND_OFFSET"


