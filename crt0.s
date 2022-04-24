; Startup code for cc65 and Shiru's NES library
; based on code by Groepaz/Hitmen <groepaz@gmx.net>, Ullrich von Bassewitz <uz@cc65.org>




; Edited to work with MMC3 code
.define SOUND_BANK 60
;segment BANK12

;MULTI_MUSIC = 3 ;Commonly used musics: 0,1,7
FT_BASE_ADR		= $0100		;page in RAM, should be $xx00
FT_DPCM_OFF		= $f000		;$c000..$ffc0, 64-byte steps
FT_SFX_STREAMS	= 1			;number of sound effects played at once, 1..4

FT_THREAD       = 1		;undefine if you call sound effects in the same thread as sound update
FT_PAL_SUPPORT	= 1		;undefine to exclude PAL support
FT_NTSC_SUPPORT	= 1		;undefine to exclude NTSC support
FT_DPCM_ENABLE  = 1		;undefine to exclude all DMC code
FT_SFX_ENABLE   = 1		;undefine to exclude all sound effects code

USE_FAMITRACKER = 0
VT_SYSTEM_TYPE = 0; 0 for VT02, 1 for VT03, 2 for VT32, 3 for VT369
VT_412C_BANKSWITCH = 0; 0 to disable, 1 to enable. Only applies to VT02 and VT03 consoles
VT_MULTICART = 1; 0 to disable, 1 to enable.
VRT_PRG_SIZE = 2048; 128 for 2MB, 256 for 4MB, 512 for 8MB, 1024 for 16MB, 2048 for 32MB, 4096 for 64MB

CB_VERSION = 0;




;REMOVED initlib
;this called the CONDES function

    .export _exit,__STARTUP__:absolute=1
	.import push0,popa,popax,_main,zerobss,copydata

; Linker generated symbols
	.import __STACK_START__   ,__STACKSIZE__ ;changed
	.import __ROM0_START__  ,__ROM0_SIZE__
	.import __STARTUP_LOAD__,__STARTUP_RUN__,__STARTUP_SIZE__
	.import	__CODE_LOAD__   ,__CODE_RUN__   ,__CODE_SIZE__
	.import	__RODATA_LOAD__ ,__RODATA_RUN__ ,__RODATA_SIZE__
	.import NES_MAPPER, NES_PRG_BANKS, NES_CHR_BANKS, NES_MIRRORING, OVERDUMP

    .include "zeropage.inc"




PPU_CTRL	=$2000
PPU_MASK	=$2001
PPU_STATUS	=$2002
PPU_OAM_ADDR=$2003
PPU_OAM_DATA=$2004
PPU_SCROLL	=$2005
PPU_ADDR	=$2006
PPU_DATA	=$2007
PPU_OAM_DMA	=$4014
PPU_FRAMECNT=$4017
DMC_FREQ	=$4010
CTRL_PORT1	=$4016
CTRL_PORT2	=$4017

OAM_BUF		=$0200
PAL_BUF		=$0600
VRAM_BUF	=$0500



.segment "ZEROPAGE"

NTSC_MODE: 			.res 1
FRAME_CNT1: 		.res 1
FRAME_CNT2: 		.res 1
VRAM_UPDATE: 		.res 1
NAME_UPD_ADR: 		.res 2
NAME_UPD_ENABLE: 	.res 1
PAL_UPDATE: 		.res 1
PAL_BG_PTR: 		.res 2
PAL_SPR_PTR: 		.res 2
OAM_VRAM_UPDATE: 	.res 1
OAM_VRAM_ADDR_HI: 	.res 1
OAM_VRAM_ADDR_LO: 	.res 1
SCROLL_X: 			.res 1
SCROLL_Y: 			.res 1
SCROLL_X1: 			.res 1
SCROLL_Y1: 			.res 1
PAD_STATE: 			.res 2		;one byte per controller
PAD_STATEP: 		.res 2
PAD_STATET: 		.res 2
PPU_CTRL_VAR: 		.res 1
PPU_CTRL_VAR1: 		.res 1
PPU_MASK_VAR: 		.res 1
RAND_SEED: 			.res 2
FT_TEMP: 			.res 3
GFX_MODE:			.res 1
VT_XGFX_MODE:       .res 1
PPU_PAL_UPD:		.res 1
PPU_PAL_UPD_SINGLE: .res 1
PPU_PAL_UPD_SIMUL:  .res 1

TEMP: 				.res 11
SPRID:				.res 1

PAD_BUF		=TEMP+1

PTR			=TEMP	;word
LEN			=TEMP+2	;word
NEXTSPR		=TEMP+4
SCRX		=TEMP+5
SCRY		=TEMP+6
SRC			=TEMP+7	;word
DST			=TEMP+9	;word

RLE_LOW		=TEMP
RLE_HIGH	=TEMP+1
RLE_TAG		=TEMP+2
RLE_BYTE	=TEMP+3

;nesdoug code requires
VRAM_INDEX:			.res 1
META_PTR:			.res 2
DATA_PTR:			.res 2


;.segment "HEADER"
.include "ines.inc"
;.if(VT_SYSTEM_TYPE = 2)
;nes2mapper 296
;.elseif (VT_412C_BANKSWITCH) .and ((VT_SYSTEM_TYPE = 0) .or (VT_SYSTEM_TYPE = 1))
;nes2mapper 270
;.else
nes2mapper 268, CB_VERSION
;.endif
.if(VRT_PRG_SIZE>=4096)
;.error "End"
nes2prg_64
.else
nes2prg NES_PRG_BANKS*16384
.endif
nes2chr 0
nes2bram 8192
nes2chrram 262144
nes2tv 'N'
.if(VT_MULTICART)
nes2exp 'M'
.else
nes2exp 'S'
.endif
;.if(VT_SYSTEM_TYPE = 0)
;nes2sys '2'
;.elseif(VT_SYSTEM_TYPE = 1)
;nes2sys '3'
;.elseif(VT_SYSTEM_TYPE = 2)
;nes2sys '9'
;.elseif(VT_SYSTEM_TYPE = 3)
;nes2sys '8'
;.else
nes2sys 'N'
;.endif
nes2end
;.incbin "ines.bin"
;.ifndef NES

;    .byte $4e,$45,$53,$1a
;	.byte <NES_PRG_BANKS
;	.byte <NES_CHR_BANKS
;	.byte <NES_MIRRORING|(<NES_MAPPER<<4)
;	.byte <NES_MAPPER&$f0+$0b
;	.byte 1 ;8k of PRG RAM
;	.byte 1
;	.byte 0
;	.byte 0
;	.byte 2
;	.byte 6
;	.byte 0
;	.byte $2a
;.endif


; linker complains if I don't have at least one mention of each bank
.segment "ONCE"

.segment "TEST_ROM"
.segment "MENU_GAME_DATA_1"
.segment "MENU_GAME_DATA_2"
.segment "MENU_GAME_DATA_3"
.segment "MENU_GAME_DATA_4"
.segment "MENU_GAME_NAMES_1"
.segment "MENU_GAME_NAMES_2"
.segment "MENU_GAME_NAMES_3"
.segment "MENU_GAME_NAMES_4"

.segment "BANK0"
.segment "BANK1"
.segment "BANK2"
.segment "BANK3"
.segment "BANK4"
.segment "BANK5"
.segment "BANK6"
.segment "BANK7"
.segment "BANK8"
.segment "BANK9"
.segment "BANK10"
.segment "BANK11"
.segment "BANK12"


.segment "STARTUP"
; this should be mapped to the last PRG bank

start:
_exit:

    sei
	cld
	ldx #$40
	stx CTRL_PORT2
    ldx #$ff
    txs
    inx
    stx PPU_MASK
    stx DMC_FREQ
    stx PPU_CTRL		;no NMI
	
	jsr _disable_irq ;disable mmc3 IRQ
	
	;x is zero

initPPU:
    bit PPU_STATUS
@1:
    bit PPU_STATUS
    bpl @1
@2:
    bit PPU_STATUS
    bpl @2

	;jmp clearVRAM
clearPalette:
	lda #$3f
	sta PPU_ADDR
	stx PPU_ADDR
	lda #$0f
	ldx #$20
@1:
	sta PPU_DATA
	dex
	bne @1
	
	
	lda #$01				; DEBUGGING
	sta PPU_DATA
	lda #$11
	sta PPU_DATA
	lda #$21
	sta PPU_DATA
	lda #$30
	sta PPU_DATA

clearVRAM:
	txa
	ldy #$20
	sty PPU_ADDR
	sta PPU_ADDR
	ldy #$10
@1:
	sta PPU_DATA
	inx
	bne @1
	dey
	bne @1

clearRAM:
    txa
@1:
    sta $000,x
    sta $100,x
    sta $200,x
    sta $300,x
    sta $400,x
    sta $500,x
    sta $600,x
    sta $700,x
    inx
    bne @1
	
; don't call any subroutines until the banks are in place	
	
	
	
; MMC3 reset
	;lda #%1000
	;sta $410b

	;.if (VT_SYSTEM_TYPE = 2)
	;lda #$00
	;sta $411d
	;sta $412c
	;.endif

	;lda #$00
	;sta $4100

; set which bank at $8000
; also $c000 fixed to 14 of 15
	lda #20 ; PRG bank zero
	jsr _set_prg_8000
; set which bank at $a000
	lda #61 ; PRG bank 13 of 15
	jsr _set_prg_a000

	lda #$80
	jsr _invert_c
	
; with CHR invert, set $0000-$03FF
	lda #0
	jsr _set_chr_mode_2
; with CHR invert, set $0400-$07FF
	lda #1
	jsr _set_chr_mode_3
; with CHR invert, set $0800-$0BFF
	lda #2
	jsr _set_chr_mode_4
; with CHR invert, set $0C00-$0FFF
	lda #3
	jsr _set_chr_mode_5
; with CHR invert, set $1000-$17FF
	lda #4
	jsr _set_chr_mode_0
; with CHR invert, set $1800-$1FFF
	lda #6
	jsr _set_chr_mode_1
;set mirroring to vertical, no good reason	
	lda #0
	jsr _set_mirroring
;allow reads and writes to WRAM, Not used for this Multicart
	;lda #$80 ;WRAM_ON 0x80
	;jsr _set_wram_mode
	
	cli ;allow irq's to happen on the 6502 chip	
		;however, the mmc3 IRQ was disabled above
	
	

	lda #4
	jsr _pal_bright
	jsr _pal_clear
	jsr _oam_clear

	lda #%00001010
	sta $2011

    jsr zerobss
	jsr	copydata

    lda #<(__STACK_START__+__STACKSIZE__) ;changed
    sta	sp
    lda	#>(__STACK_START__+__STACKSIZE__)
    sta	sp+1            ; Set argument stack ptr

;	jsr	initlib
; removed. this called the CONDES function





	








	lda #%10000000
	sta <PPU_CTRL_VAR
	sta PPU_CTRL		;enable NMI
	lda #%00000110
	sta <PPU_MASK_VAR

waitSync3:
	lda <FRAME_CNT1
@1:
	cmp <FRAME_CNT1
	beq @1

detectNTSC:
	ldx #52				;blargg's code
	ldy #24
@1:
	dex
	bne @1
	dey
	bne @1

	lda PPU_STATUS
	and #$80
	sta <NTSC_MODE

	jsr _ppu_off

	lda #0
	ldx #0
	jsr _set_vram_update

	lda #$fd
	sta <RAND_SEED
	sta <RAND_SEED+1

	lda #0
	sta PPU_SCROLL
	sta PPU_SCROLL
	
	
	
;BANK12
.if (USE_FAMITRACKER)
	lda #SOUND_BANK ;swap the music in place before using
					;SOUND_BANK is defined above
	jsr _set_prg_8000
	
	ldx #<music_data
	ldy #>music_data
	lda <NTSC_MODE
	jsr FamiToneInit

	ldx #<sounds_data
	ldy #>sounds_data
	jsr FamiToneSfxInit
	
	lda #$00 ;PRG bank #0 at $8000, back to basic
	jsr _set_prg_8000
.endif	
	; Initialize LCD display
	
	; end LCD initialization
	
	; Init Multi Music

	; end Multi Music

	jmp _main			;no parameters
	

	.include "MMC3/mmc3_code.asm"
	.include "LIB/neslib.s"
	.include "LIB/nesdoug.s"
	



	

	
.segment "CODE"	
.if(USE_FAMITRACKER)
	.include "MUSIC/famitone2.s"
.endif
; When music files get very big, it's probably best to
; split the songs into multiple swapped banks
; the music code itself is in the regular CODE banks.
; It could be moved into BANK12 if music data is small.
	
.segment "BANK4"	
	
music_data:
.if(USE_FAMITRACKER)
	.include "MUSIC/TestMusic3.s"
.endif

sounds_data:
.if(USE_FAMITRACKER)
	.include "MUSIC/SoundFx.s"
.endif


	
	
;.segment "SAMPLES"
;	.incbin "MUSIC/BassDrum.dmc"



.segment "VECTORS"

    .word nmi	;$fffa vblank nmi
    .word start	;$fffc reset
   	.word irq	;$fffe irq / brk

.segment "CHARS"
	.include "Chars.inc"
; the CHARS segment is much bigger, and I could have 
; incbin-ed many more chr files

.if(VT_SYSTEM_TYPE = 2)
.segment "MUSIC"
	.include "Sounds.inc"	
.endif

.include "Music.inc"

.segment "STARTUP"
	.include "Palettes.inc"

.include "data_def.inc"
.include "cart_data.inc"