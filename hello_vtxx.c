
#include "LIB/neslib.h"
#include "LIB/nesdoug.h" 
#include "MMC3/mmc3_code.h"
#include "MMC3/mmc3_code.c"

#pragma bss-name(push, "ZEROPAGE") // Global variables
unsigned char pad1;
unsigned char pad1_new;

#pragma bss-name(pop) // Variable that were on 0x300
unsigned char irq_array[32];
unsigned char double_buffer[32];
unsigned char counter;

#pragma bss-name(push, "XRAM") // WorkRam
unsigned char wram_array[0x2000];

#pragma bss-name(pop) // Variable that were on 0x300

const unsigned char palette_bg[]={
0x0f, 0x30, 0x10, 0x30,
0x0f, 0x23, 0, 0,
0x0f, 0, 0, 0,
0x0f, 0, 0, 0
}; 

const unsigned char palette_spr[]={
0x0f, 0x12, 0x12, 0x32,
0x0f, 0x12, 0x12, 0x12,
0x0f, 0x12, 0x12, 0x12,
0x0f, 0x12, 0x12, 0x12
}; 

unsigned char text[] = "Hello World!";
#pragma rodata-name ("CODE")
#pragma code-name ("CODE")	
//extern unsigned char menu_mus[];

void main (void) {    
	set_mirroring(MIRROR_VERTICAL);            
	bank_spr(1);	
    disable_irq();
    irq_array[0] = 0xff; // end of data
		
	// clear the WRAM, not done by the init code
	// memfill(void *dst,unsigned char value,unsigned int len);
	memfill(wram_array,0,0x2000); 
	
	ppu_off(); // screen off

	pal_bg(palette_bg);

    vram_adr(NTADR_A(4,4));
    vram_write(text, sizeof(text));

    ppu_on_all();
	
	while (1){ // infinite loop
		ppu_wait_nmi();
	}
}