LOADROM_OFFSET = $0300
GFXLOAD_OFFSET = $0200
LOADROM_INIT_BITS = $ff
VBLANK_WAIT_TIME = 0
.if(CB_VERSION = 1)
CB_REG0 = $5000
CB_REG1 = $5001
CB_REG2 = $5002
CB_REG3 = $5003
.else
CB_REG0 = $6000
CB_REG1 = $6001
CB_REG2 = $6002
CB_REG3 = $6003
.endif

.define	db	.byte
.define	dw	.addr
.define dd	.faraddr
.define VBANK16(no)  .LOBYTE(no>>11)
.define VBANK4(no)  .LOBYTE(no>>10)
.define VBANK16DATA(no)  .LOBYTE(no>>11),.LOBYTE(no>>11)+1,.LOBYTE(no>>11)+2,.LOBYTE(no>>11)+3
.define VBANK4DATA(no)  .byte	.LOBYTE(no>>10),.LOBYTE(no>>10)+1,.LOBYTE(no>>10)+2,.LOBYTE(no>>10)+3

.macro	assign begin,this,num       
	.if(.LOBYTE(this - begin) & (num - 1))
	.res	(num - (.LOBYTE(this- begin) & (num - 1))),$FF
	.endif
.endmacro	
	
;.macro bind_vrom vrom,bpp
;db (.LOBYTE(bpp << 4) | .LOBYTE((((vrom >> 13) & $20) * 8) & $0f))
;.endmacro

;bittmp = 0
;bittmp2 = 0
;bittmp3 = 0

.macro gametable prom,prom_end,vrom,vrom_end,nrom,mapper
    ; PRG Section
	.if ((prom_end - prom) <= $20000) ; Less than 128k of ROM?
        .if ((vrom_end - vrom) <= $20000) ; Less than 128k of CHR?
        db %11000000 | ((.LOBYTE(prom >> 23) & $03) << 4) | (.LOBYTE(prom >> 17) & $07)
        .else
        db %01000000 | ((.LOBYTE(prom >> 23) & $03) << 4) | (.LOBYTE(prom >> 17) & $07)
        .endif  
    .else
        .if ((vrom_end - vrom) <= $20000) ; Less than 128k of CHR?
        db %10000000 | ((.LOBYTE(prom >> 23) & $03) << 4) | (.LOBYTE(prom >> 17) & $07)
        .else
        db %00000000 | ((.LOBYTE(prom >> 23) & $03) << 4) | (.LOBYTE(prom >> 17) & $07)
        .endif        
    .endif    
    
    .if (((prom_end - prom) <= $4000) && mapper = 1)
    db %10000000 | (.LOBYTE((prom >> 20) & $01) << 4) | (.LOBYTE((prom >> 22) & $01) << 3) | (.LOBYTE((prom >> 21) & $01) << 2) | %00
    .elseif ((prom_end - prom) <= $40000)
    db %10000000 | (.LOBYTE((prom >> 20) & $01) << 4) | (.LOBYTE((prom >> 22) & $01) << 3) | (.LOBYTE((prom >> 21) & $01) << 2) | %10
    .else
    db %00000000 | (.LOBYTE((prom >> 20) & $01) << 4) | (.LOBYTE((prom >> 22) & $01) << 3) | (.LOBYTE((prom >> 21) & $01) << 2) | %10
    .endif

    .if (mapper = 1)
    db %10010000 | ((.LOBYTE(prom >> 14) & $07) << 1)
    .else
    db %10000000
    .endif

    db .LOBYTE(prom >> 14)
    ; End PRG Section
    ; CHR Section
    .if ((vrom_end - vrom) <= $20000) ; Less than 128k of CHR?
    db %11000000 | ((.LOBYTE(vrom >> 23) & $03) << 4) | (.LOBYTE(vrom >> 17) & $07)
    .else
    db %00000000 | ((.LOBYTE(vrom >> 23) & $03) << 4) | (.LOBYTE(vrom >> 17) & $07)
    .endif
    db %10000000 | (.LOBYTE((vrom >> 20) & $01) << 4) | (.LOBYTE((vrom >> 22) & $01) << 3) | (.LOBYTE((vrom >> 21) & $01) << 2)
    db %00000000

    db .LOBYTE(vrom >> 13)
    db .LOBYTE((vrom_end - vrom) / $2000)
    ;db .HIBYTE((vrom_end - vrom))    
    ; End CHR Section
.endmacro	

.segment "STARTUP"    
    LoadSub:
        rts
    LoadGame:        
        .export _StartGame = LoadGame             
        sta $80
        stx $81         
        
        ; Load
        LDX #$00
        STX $2001

        STX $2000       
        lda #$00

        ldx #$00
        txa
        @APU_LOOP:
        STA $4000,x
        INX
        CPX #$14
        BNE @APU_LOOP

        lda #$00
        STA $4015
        
        LDX #$00
        STX $2010
        stX $2012        

        LDA #LOADROM_INIT_BITS
        ldx #$00        
    @ClearRAM_M:
        ;sta $0000,x
        sta $0100,x
        sta $0200,x
        sta $0300,x
        sta $0400,x
        sta $0500,x
        sta $0600,x
        sta $0700,x
        inx
        bne @ClearRAM_M

        LDA #$00
        STA $2005
        STA $2005

        LDA #$00
        STA $2006
        STA $2006
    
        ; Loadd

        LDX #%00000000
        ;STX $4105

        ldx #$00
        STX $2010
        STX $2005
        STX $2005
        STX $2006
        STX $2005
        STX $2003
        STX $2001
        STX $2000
        STX $2011
        STX $4103  

        ldy #$04
        jsr WAIT_VBLANK      
    @0:        
        lda LOADROM_START,X
        STA LOADROM_OFFSET, X
        inx
        cpx #(LOADROM_END-LOADROM_START)
        bcc @0

        ldy #$00
        ldx #$00
    @0_gfx:        
        lda GFX_LOADER_START,X
        STA GFXLOAD_OFFSET, X
        inx
        cpx #(GFX_LOADER_END-GFX_LOADER_START)
        bcc @0_gfx
        ldy #$00        
    @1:
        lda ($80), y
        sta $0400, y
        iny
        cpy #$10
        bcc @1
        ldy #$00

        lda $040A
        sta LOADROM_OFFSET+1
        lda $040B
        sta LOADROM_OFFSET+2

        ldx #$00        
        lda #LOADROM_INIT_BITS
    @ClearRAM_M_2:
        sta $0000,x
        inx
        bne @ClearRAM_M_2
        
        ldy #$04
        jsr WAIT_VBLANK
        jmp LOADROM

    GFX_LOADER_START:
    .org GFXLOAD_OFFSET
    GFX_LOADER:
    ; GFX Loader
    LDX $0409
    CPX #$00
    BEQ @END_GFX_LOADER
    LDX $0405
    STX CB_REG0
    LDX $0406
    STX CB_REG1
    LDX #$00
    STX $12
    STX CB_REG2
    LDX $0407
    STX CB_REG3
    @GFX_RELOAD:
    LDX $0408
    STX $11
    LDX $0409
    ;CPX #$01
    ;BCS @CHR_ROM_LOADER
    ;LDX #$10
    @CHR_ROM_LOADER:
    STX $10

    @GFX_LOAD_LOOP:        
    LDA $12
    JSR LOADROM_BANKSWITCH        
    LDY #$06
    STY $8000
    LDA $11
    STA $8001
    LDX #$80
    STX $01
    LDX #$00
    STX $00
    STX $2006
    STX $2006               
    LDY #$00

    @GFX_LOAD_DATA_LOOP:
    LDA ($00), Y
    STA $2007
    INY
    BNE @GFX_LOAD_DATA_LOOP
    .if(VBLANK_WAIT_TIME)
    ldx #VBLANK_WAIT_TIME
    jsr disp_vblank    
    .endif
    INC $01
    LDA $01
    CMP #$A0
    BNE @GFX_LOAD_DATA_LOOP

    @GFX_LOAD_DATA_END:
    INC $12
    DEC $10
    BEQ @GFX_LOAD_END
    INC $11                    
    JMP @GFX_LOAD_LOOP
    @GFX_LOAD_END:
    LDA $040C
    AND #$10
    BNE @END_GFX_LOADER
    LDA $12
    CMP #$10
    BCC @GFX_RELOAD
    @END_GFX_LOADER:
    rts
    ; End GFX Loader
    .reloc
    GFX_LOADER_END:

        
    LOADROM_START:
    .org LOADROM_OFFSET
    LOADROM:
        ; Load Routines
        jsr $ffff
        jmp LOADROM_BEGIN
    LOADROM_BANKSWITCH:
        ldx #$00

        asl a
        asl a
        asl a
        tay        

        stx $8000 ; 0
        sty $8001

        inx
        iny
        iny

        stx $8000 ; 1
        sty $8001

        inx
        iny
        iny

        stx $8000 ; 2
        sty $8001

        inx
        iny

        stx $8000 ; 3
        sty $8001

        inx
        iny

        stx $8000 ; 4
        sty $8001

        inx
        iny

        stx $8000 ; 5
        sty $8001
        rts           
    disp_vblank:        
        bit $2002
    @vblank_l:        
        lda $2002
        bpl @vblank_l
        dex
        bne disp_vblank
        rts                              
    LOADROM_BEGIN:
   
        ;ldx #$96
        ;stx $2010
        ; Initialize PPU
        ;ldx $040a
        ldx #$00
        stx $a001

        ;lda #$00
        ;jsr LOADROM_BANKSWITCH

        ; end Initialize PPU

        JSR GFX_LOADER

        lda #$00
        jsr LOADROM_BANKSWITCH

        LDX $0400
        STX $A000

        LDX $0401
        STX CB_REG0

        LDX $0402
        STX CB_REG1

        LDX #$00
        STX CB_REG2
        STX $2006
        STX $2006
        STX $2005
        STX $2005

        LDX $0403
        STX CB_REG3

        ; Not needed for TLROM games
        ;LDX #$80
        ;STX $A001

        LDY #$06
        STY $8000

        LDX $0404
        STX $8001

        INY
        STY $8000

        INX
        STX $8001
                
        LDX #$08
        JSR disp_vblank

        LDX #$00        
    @LOADROM_RAMCLR_2:
        lda #$00
        sta $0000,x
        lda #LOADROM_INIT_BITS
        sta GFXLOAD_OFFSET,x
        sta $0400,x
        inx        
        bne @LOADROM_RAMCLR_2

    @LOADROM_WIPE:
        ldx #$00
        lda #LOADROM_INIT_BITS
    @LOADROM_WIPE_LOOP:
        sta LOADROM_OFFSET,x
        inx
        cpx #(@LOADROM_WIPE_LOOP-LOADROM)
        bcc @LOADROM_WIPE_LOOP

        ldx #$00
		lda #$00
		ldy #$00

        STX $2006
        STX $2006
        STX $2005
        STX $2005

        JMP ($FFFC) ; Reset      
    .reloc
    LOADROM_END:    
    WAIT_VBLANK:        
        bit $2002
    @vblank_w:        
        lda $2002
        bpl @vblank_w
        dey
        bne WAIT_VBLANK
        rts