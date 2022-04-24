.export  _MultiMusic_Update, _MultiMusic_Play, _PlaySnd
;.export _MenuSCR_PAL

.segment "STARTUP"
;_MenuSCR_PAL:
;.incbin "RSC\SG150\MenuScreen.pal"


.segment "MENU_GAME_NAMES_1"
.incbin "names.bin"

.segment "MENU_GAME_DATA_1"
.include "games.inc"

.segment "STARTUP"
;_Menu_Tables:


_MultiMusic_Update:
    ;jsr $d1a1
    rts

_MultiMusic_Play:
    ;jsr $d1d0
	rts

_PlaySnd:
    ;jsr $d1a8
    lda #$0f
    sta $4015
    lda #$8f
    sta $4000 
    lda #$aa
    sta $4001
    sta $4002
    lda #$80
    sta $4003   
;    AND #$01
;    BNE @SND2
;@SND1:
;    LDA #$03
;    STA $4015
;    LDA #$87
;    STA $4000
;    LDA #$89
;    STA $4001
;    LDA #$F0
;    STA $4002
;    LDA #$00
;    STA $4003
    rts
;@SND2:
;    LDA #$02
;    STA $4015
;    LDA #$3F
;    STA $4004
;    LDA #$9A
;    STA $4005
;    LDA #$FF
;    STA $4006
;    LDA #$00
;    STA $4007
;    rts