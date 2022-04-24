
#include "LIB/neslib.h"
#include "LIB/nesdoug.h" 
#include "MMC3/mmc3_code.h"
#include "MMC3/mmc3_code.c"
#include "Resources/menu_texts.h"
#include "games.h"
#include "settings.h"

#define STA_SFX 20
//#define STA_SFX 34
#define SEL_SFX 19
//#define SFX 1
#define MUS 0

#define B_X_OFFS 7
#define B_Y_OFFS 3

#define TEXT_OFFSET 10
#define TEXT_LENGTH 24
#define M_X 48
#define M_Y (TEXT_OFFSET*8+2)

#define ROM_BANK 0x39
#define ROM_NAMES 0x3b
#define ROM_OFFSETS 0x38
#define ROM_CHR 0x00
#define ROM_NAMETABLE 0x34
#define ARR_OFFSET 0xb6
#define CHARA_OFFSET 0x80

#pragma bss-name(push, "ZEROPAGE") // Global variables
unsigned char pad1;
unsigned char pad1_new;
unsigned char text_offs;
unsigned char dig_offset;
//unsigned char p_offset;
unsigned char cart_inserted;
unsigned int game_offset;
unsigned int gfx_offset;
unsigned char gfx_bank_offset;
unsigned char title_buffer[4];
unsigned int sram_offset;
unsigned char checking_sram;
unsigned char sram_expected;

#pragma bss-name(pop) // Variable that were on 0x300
unsigned char irq_array[64];
//unsigned char double_buffer[64];
unsigned char counter;
unsigned char line;
unsigned char delay_s;
unsigned int tmp_digit;
unsigned char digits[8] = {0x30, 0x30, 0x30, 0x2e,0,0,0,0};
int menu_offset;
unsigned char menu_hold_delay;
unsigned char page;
unsigned char m_d;
unsigned char ar_pos;
unsigned char ar_del;
unsigned char text_tmp[64];
unsigned char text_tmp_i[64];
unsigned char x_pos_t;
unsigned char x_pos_tl;
unsigned char x_pos_ch;
unsigned int MAXGAME = 200;
unsigned int rom_shift = 0;
unsigned int rom_name_offset;
//unsigned char m_txt[5];
//unsigned int game_offset;

unsigned char bank_temp_8000;

unsigned char d_cat;
unsigned char s_delay = 0x8;
/*
#define CART1_GAMES 200
#define CART2_GAMES 200
#define MAXGAMES ((cart_inserted ? CART2_GAMES : CART1_GAMES) / MAXLINES)
#define MAXGAME (cart_inserted ? CART2_GAMES : CART1_GAMES)
*/

//#define MAXGAME CART_GAMES
//#define TESTROM ROMS_MAX
#define TESTROM 0
#define MAXGAMES MAXGAME / MAXLINES

#define CART_GAMES 200
#define MAXGAMES_(x) x / MAXLINES
#define MAXLINES 8

#pragma bss-name(push, "XRAM") // WorkRam
//unsigned char wram_array[0x2000];

#pragma bss-name(pop) // Variable that were on 0x300

/*
const unsigned char palette_bg[]={
0x0f, 0x30, 0x10, 0x30,
0x0f, 0x21, 0, 0,
0x0f, 0, 0, 0,
0x0f, 0, 0, 0
}; 

const unsigned char palette_bg_a1[]={
0x0f, 0x30, 0x10, 0x30,
0x0f, 0x24, 0, 0,
0x0f, 0, 0, 0,
0x0f, 0, 0, 0
}; 

const unsigned char palette_spr[]={
0x0f, 0x15, 0x25, 0x30,
0x0f, 0x0f, 0x0f, 0x0f,
0x0f, 0x0f, 0x0f, 0x0f,
0x0f, 0x0f, 0x0f, 0x0f,
}; 
*/

const unsigned char MenuPal[] = {
	0x0f, 0x30, 0x1a, 0x04,
	0x0f, 0x11, 0x1a, 0X04,
	0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f,
};

const unsigned char MenuPal_S[] = {
	0x01, 0x13, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f,
	0x0f, 0x0f, 0x0f, 0x0f,
};

const unsigned char IDENT[] = "Developed using OpenMultiLoader, Version 0.1";
const unsigned char COMPILE_DATE[] = __DATE__;
const unsigned char COMPILE_TIME[] = __TIME__;
//const unsigned char COMPILER[]     = __CC65__;

//const unsigned char menu[] = "200 IN 1 SG101";
//const unsigned char menu_2[] = "200 IN 1 SG102";
#pragma rodata-name ("CODE")
#pragma code-name ("CODE")	
/*
extern const unsigned char start_menu[];
extern const unsigned char start_palette[];
*/
//extern const unsigned char start_palette_2[];
const unsigned char blank[32] = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

const unsigned char startup_attr[64] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};


void DrawPage(unsigned char page, unsigned char fast);
void DrawText(unsigned char *text, unsigned char length, int address, unsigned char fast);
void StartGame(int offset);
void TestMode();
void MultiMusic_Play(unsigned char mus);
void PlaySnd(unsigned char snd);

/*
extern const unsigned char StartSplash_NT[];
extern const unsigned char Menu_Tables[];

extern const unsigned char MenuSCR_NT[];
extern const unsigned char MenuSCR_PAL[];
*/

unsigned char IsCartridgeInserted() {	
	return !(PEEK(0x412d) & 0x1);
};

void ClearCHRBlock();

void RunGame(unsigned int game) {	
	game += rom_shift;
	set_prg_8000(ROM_OFFSETS);
	game_offset = (PEEK( 0x8001+(2*game) ) << 8) | PEEK( 0x8000+(2*game));

	set_extended_gfx(0);
	pal_clear();
	oam_clear();

	ppu_wait_nmi();
	//while (!is_irq_done()) {}
	ppu_off();
	disable_irq();			

	vram_adr(NTADR_A(0,0));
	vram_fill(0x00, 0x1000);			

	//memfill(wram_array,0,0x2000); 
	
	set_prg_8000(game >= 682 ? ROM_BANK+1 : ROM_BANK);
	ClearCHRBlock();
	StartGame(game_offset);
}

/*void draw_title(unsigned int offset, unsigned char *text, unsigned char textlen) {
	
}*/

/*
void draw_title() {
	for (x_pos_t = 0; x_pos_t<3; x_pos_t++) {
		set_vram_buffer();
		multi_vram_buffer_vert(Numbers[2]+(3*x_pos_t), 3, NTADR_A(B_X_OFFS+x_pos_t, B_Y_OFFS));
		ppu_wait_nmi();
	}
	for (x_pos_t = 0; x_pos_t<3; x_pos_t++) {
		set_vram_buffer();
		multi_vram_buffer_vert(Numbers[0]+(3*x_pos_t), 3, NTADR_A(B_X_OFFS+x_pos_t+3, B_Y_OFFS));
		ppu_wait_nmi();
	}
	for (x_pos_t = 0; x_pos_t<3; x_pos_t++) {
		set_vram_buffer();
		multi_vram_buffer_vert(Numbers[0]+(3*x_pos_t), 3, NTADR_A(B_X_OFFS+x_pos_t+6, B_Y_OFFS));
		ppu_wait_nmi();
	}		

	for (x_pos_t = 0; x_pos_t<4; x_pos_t++) {
		set_vram_buffer();
		multi_vram_buffer_vert(NUM_IN+(3*x_pos_t), 3, NTADR_A(B_X_OFFS+x_pos_t+10, B_Y_OFFS));
		ppu_wait_nmi();
	}		

	for (x_pos_t = 0; x_pos_t<2; x_pos_t++) {
		set_vram_buffer();
		multi_vram_buffer_vert(Numbers[1]+(3*x_pos_t), 3, NTADR_A(B_X_OFFS+x_pos_t+15, B_Y_OFFS));
		ppu_wait_nmi();
	}	
}
*/

/*
void draw_title(unsigned char *text, unsigned char textlen) {	
	x_pos_tl = 0;
	for (x_pos_ch = 0; x_pos_ch<textlen; x_pos_ch++) {
		if (text[x_pos_ch] == 0x2d) {
			x_pos_tl += 1;
			for (x_pos_t = 0; x_pos_t<4; x_pos_t++) {
				set_vram_buffer();
				multi_vram_buffer_vert(NUM_IN+(3*x_pos_t), 3, NTADR_A(B_X_OFFS+x_pos_t+x_pos_tl, B_Y_OFFS));
				flush_vram_update2();
			}
			x_pos_tl += 5;
		} else if (text[x_pos_ch] >= 0x30 && text[x_pos_ch] <= 0x39) {
			for (x_pos_t = 0; x_pos_t<Numbers_Length[text[x_pos_ch]-0x30]; x_pos_t++) {
				set_vram_buffer();
				multi_vram_buffer_vert(Numbers[text[x_pos_ch]-0x30]+(3*x_pos_t), 3, NTADR_A(B_X_OFFS+x_pos_t+x_pos_tl, B_Y_OFFS));				
				flush_vram_update2();
			}
			x_pos_tl += Numbers_Length[text[x_pos_ch]-0x30];
		} else {
			for (x_pos_t = 0; x_pos_t<Numbers_Length[10]; x_pos_t++) {
				set_vram_buffer();
				multi_vram_buffer_vert(Numbers[10]+(3*x_pos_t), 3, NTADR_A(B_X_OFFS+x_pos_t+x_pos_tl, B_Y_OFFS));				
				flush_vram_update2();
			}
			x_pos_tl += Numbers_Length[10];
		}
	}
	//ppu_wait_nmi();
}
*/

void render_title() {
	for (x_pos_ch = 0; x_pos_ch < 11; x_pos_ch++) {
		set_vram_buffer();
		for (x_pos_t = 0; x_pos_t<4; x_pos_t++) {
			title_buffer[x_pos_t] = (4*x_pos_ch)+x_pos_t;
		}
		multi_vram_buffer_vert(title_buffer, 4, NTADR_A(B_X_OFFS+x_pos_ch, B_Y_OFFS));
		flush_vram_update2();
	}
	for (x_pos_ch = 0; x_pos_ch < 4; x_pos_ch++) {
		set_vram_buffer();
		for (x_pos_t = 0; x_pos_t<4; x_pos_t++) {
			title_buffer[x_pos_t] = (4*x_pos_ch)+x_pos_t+0x2c;
		}
		multi_vram_buffer_vert(title_buffer, 4, NTADR_A(B_X_OFFS+x_pos_ch+12, B_Y_OFFS));
		flush_vram_update2();
	}	
	vram_adr(0x23c0);
	vram_write(startup_attr, sizeof(startup_attr));
};

#if CATEGORY_MENU
void render_menu(unsigned char page) {
	bank_temp_8000 = get_prg_8000();
	set_prg_8000(ROM_NAMETABLE);
	//pal_clear();
	//ppu_wait_nmi();

	ppu_off();	
	pal_update_simul();	
	disable_irq();

	set_chr_mode_2(Menu_Tables[page]+ROM_CHR);
	set_chr_mode_3(Menu_Tables[page]+1+ROM_CHR);
	set_chr_mode_4(Menu_Tables[page]+2+ROM_CHR);
	set_chr_mode_5(Menu_Tables[page]+3+ROM_CHR);

	vram_adr(NTADR_A(0,0));	

	switch (page) {
		case 0:
			vram_write(Menu1_NT, 0x400);
			pal_bg(Menu1_palette_lower);
			pal_bg_high(Menu1_palette_upper);
			memcpy(irq_array, Menu1_irq, 48);
			break;
		case 1:
			vram_write(Menu2_NT, 0x400);
			pal_bg(Menu2_palette_lower);
			pal_bg_high(Menu2_palette_upper);
			memcpy(irq_array, Menu2_irq, 48);		
			break;
		case 2:
			vram_write(Menu3_NT, 0x400);
			pal_bg(Menu3_palette_lower);
			pal_bg_high(Menu3_palette_upper);
			memcpy(irq_array, Menu3_irq, 48);		
			break;
		case 3:
			vram_write(Menu4_NT, 0x400);
			pal_bg(Menu4_palette_lower);
			pal_bg_high(Menu4_palette_upper);
			memcpy(irq_array, Menu4_irq, 48);		
			break;
		case 4:
			vram_write(Menu5_NT, 0x400);
			pal_bg(Menu5_palette_lower);
			pal_bg_high(Menu5_palette_upper);
			memcpy(irq_array, Menu5_irq, 48);		
			break;
		case 5:
			vram_write(Menu6_NT, 0x400);
			pal_bg(Menu6_palette_lower);
			pal_bg_high(Menu6_palette_upper);
			memcpy(irq_array, Menu6_irq, 48);		
			break;
		case 6:
			vram_write(Menu7_NT, 0x400);
			pal_bg(Menu7_palette_lower);
			pal_bg_high(Menu7_palette_upper);
			memcpy(irq_array, Menu7_irq, 48);		
			break;
		case 7:
			vram_write(Menu8_NT, 0x400);
			pal_bg(Menu8_palette_lower);
			pal_bg_high(Menu8_palette_upper);
			memcpy(irq_array, Menu8_irq, 48);		
			break;
	}

	set_irq_ptr(irq_array);
	set_prg_8000(bank_temp_8000);

	ppu_on_all();

	s_delay = 0x8;
	while (s_delay) {
		s_delay--;
		ppu_wait_nmi();
	}	
}
#endif

void ClearCHRBlock() {	
	vram_adr(0x00);
	for (gfx_offset = 0; gfx_offset<0x2000; gfx_offset++) {		
		//vram_write(&PEEK(0x8000+gfx_offset), 1);
		POKE(0x2007, 0xff);
		//POKE(0x2007, 0xff);
	}
}

void ClearCHR() {	
	for (gfx_bank_offset = 0; gfx_bank_offset<32; gfx_bank_offset++) { 				
		set_chr_mode_0(8*gfx_bank_offset);
		set_chr_mode_1(8*gfx_bank_offset+2);
		set_chr_mode_2(8*gfx_bank_offset+4);
		set_chr_mode_3(8*gfx_bank_offset+5);
		set_chr_mode_4(8*gfx_bank_offset+6);
		set_chr_mode_5(8*gfx_bank_offset+7);
		vram_adr(0x00);
		for (gfx_offset = 0; gfx_offset<0x2000; gfx_offset++) {		
			//vram_write(&PEEK(0x8000+gfx_offset), 1);
			POKE(0x2007, 0xff);
			//POKE(0x2007, 0xff);
		}
	}

	s_delay = 0x8;
	while (s_delay) {
		s_delay--;
		ppu_wait_nmi();
	}	

	set_chr_mode_0(0);
	set_chr_mode_1(2);
	set_chr_mode_2(4);
	set_chr_mode_3(5);
	set_chr_mode_4(6);
	set_chr_mode_5(7);
}

void LoadGFX(int bank_id, int size) {	
	for (gfx_bank_offset = 0; gfx_bank_offset<size; gfx_bank_offset++) { 		
		set_prg_8000(bank_id+gfx_bank_offset);
		set_chr_mode_0(8*gfx_bank_offset);
		set_chr_mode_1(8*gfx_bank_offset+2);
		set_chr_mode_2(8*gfx_bank_offset+4);
		set_chr_mode_3(8*gfx_bank_offset+5);
		set_chr_mode_4(8*gfx_bank_offset+6);
		set_chr_mode_5(8*gfx_bank_offset+7);
		vram_adr(0x00);
		for (gfx_offset = 0; gfx_offset<0x2000; gfx_offset++) {		
			//vram_write(&PEEK(0x8000+gfx_offset), 1);
			POKE(0x2007, PEEK(0x8000+gfx_offset));
			//POKE(0x2007, 0xff);
		}
	}

	set_chr_mode_0(0);
	set_chr_mode_1(2);
	set_chr_mode_2(4);
	set_chr_mode_3(5);
	set_chr_mode_4(6);
	set_chr_mode_5(7);
}

void LoadCat(char cat) {
	d_cat = cat;
	MAXGAME = CAT_NO[d_cat];
	rom_shift = CAT_SHF[d_cat];
	DrawText(CAT_TXT[d_cat], 8, NTADR_A(11, 7), 1);
	DrawPage(0, 1);			
	menu_offset = 0;
	page = 0;
	m_d = 0;
}

void main (void) {    
	set_mirroring(MIRROR_VERTICAL);            

	set_prg_8000(ROM_NAMETABLE+1);
	bank_spr(0);
	bank_bg(0);

	//POKE(0x410b, 0x80); // MMC3-like Scanlines

	set_chr_mode_2(ROM_CHR);
	set_chr_mode_3(ROM_CHR+1);
	set_chr_mode_4(ROM_CHR+2);
	set_chr_mode_5(ROM_CHR+3);

    disable_irq();
    irq_array[0] = 0xff; // end of data	

	#if SPLASH_SCREEN
	memcpy(irq_array, StartSplash_irq, 48);
	set_irq_ptr(irq_array);
	#endif
		
	//cart_inserted = IsCartridgeInserted();
	//cart_inserted = 1;
	//cart_inserted = 1;

	// clear the WRAM, not done by the init code
	// memfill(void *dst,unsigned char value,unsigned int len);
	//memfill(wram_array,0,0x2000); 
	memfill(digits, 0, 0x8);
	
	ppu_off(); // screen off
	set_extended_gfx(X_GFX);
	ClearCHR();
	LoadGFX(0x20, 1);

	pad1 = pad_poll(0);

	if ((pad1 & (PAD_A|PAD_B)) == (PAD_A|PAD_B)) {
		TestMode();
	}	
	//MultiMusic_Play(0);
	PlaySnd(0);

	//MultiMusic_Play(MUS);
	//MultiMusic_Play(cart_inserted ? 1 : 0);
	#if SPLASH_SCREEN
	pal_bg(StartSplash_palette_lower);
	pal_bg_high(StartSplash_palette_upper);
	pal_spr(palette_spr);

	vram_adr(NTADR_A(0,0));
	vram_write(StartSplash_NT, 0x400);

	pal_update_simul();
	ppu_on_all();
	while (1) {
		ppu_wait_nmi();
		pad1 = pad_poll(0);
		pad1_new = get_pad_new(0);
		if (s_delay) s_delay--;
		if ((pad1_new & PAD_START) && !s_delay) break;
		while (!is_irq_done()) {}
	}	
	#endif

	#if CATEGORY_MENU
	render_menu(0);

	while (1) {
		ppu_wait_nmi();
		pad1 = pad_poll(0);
		pad1_new = get_pad_new(0);
		if (pad1 & PAD_UP) {
			PlaySnd(0);
			if (d_cat <= 0) {
				d_cat = 7;
			} else {
				d_cat--;
			}
			while (!is_irq_done()) {}
			render_menu(d_cat);
		}

		if (pad1 & PAD_DOWN) {
			PlaySnd(0);
			d_cat = (d_cat + 1) % 8;
			while (!is_irq_done()) {}
			render_menu(d_cat);
		}

		if (pad1_new & PAD_START) {
			PlaySnd(1);
			while (pad1 & PAD_START) {pad1 = pad_poll(0);}
			set_extended_gfx(0);
			pal_clear();
			ppu_wait_nmi();
			ppu_off();
			disable_irq();			
			vram_adr(NTADR_A(0,0));
			vram_fill(0x0, 0x1000);
			break;
		}
		while (!is_irq_done()) {}
	}
	#else
	d_cat = DEFAULT_CATEGORY;
	#endif
	//vram_adr(NTADR_A(12,4));
	//DrawText(cart_inserted ? menu_2 : menu, sizeof(menu), NTADR_A(9,2), 1);	
		
	//set_prg_8000(ROM_NAMETABLE+1);	
	#if 0
	POKE(0x2018, 0x00);
	POKE(0x201a, 0x00);
	#endif

	#if 0
	set_chr_mode_0(0x3c);	

	set_chr_mode_2(0x38);
	set_chr_mode_3(0x39);
	set_chr_mode_4(0x3a);
	set_chr_mode_5(0x3b);
	#endif
	
	#if 0
	disable_irq();	
	irq_array[0] = 0xf9;
	irq_array[1] = 0x38;
	irq_array[2] = 0xfa;
	irq_array[3] = 0x39;

	irq_array[4] = 0x30;

	irq_array[5] = 0xf9;
	irq_array[6] = 0x3c;
	irq_array[7] = 0xfa;
	irq_array[8] = 0x3d;	

	irq_array[9] = 0xff;	
	#endif
	
	vram_adr(NTADR_A(0,0));
	//vram_write(MenuSCR_NT, 0x400);
	vram_fill(0x80, 0x3c0);	

	/*
	vram_adr(NTADR_B(0,0));
	vram_fill(0x80, 0x400);
	*/
	pal_bg(MenuPal);
	
	//draw_title(CAT_TXT[d_cat], 5);
	render_title();
	//DrawText("300 IN 1", 8, NTADR_A(11, 7), 1);
	LoadCat(0);
	
	ppu_on_all();

	pal_spr(MenuPal_S);
	
	while (1){ // infinite loop
		ppu_wait_nmi();
		oam_clear();
		oam_spr(M_X, M_Y+(16*(menu_offset%MAXLINES)), ARR_OFFSET, 0x0);
		oam_spr(M_X+8, M_Y+(16*(menu_offset%MAXLINES)), ARR_OFFSET+1, 0x0);

		pad1 = pad_poll(0);
		pad1_new = get_pad_new(0);

		#if SELECT_CATEGORY
		if (pad1_new & PAD_SELECT) {
			ppu_off();
			d_cat = (d_cat + 1) & 1;
			LoadCat(d_cat);
			ppu_on_all();
			PlaySnd(0);
		}
		#endif

		if (pad1_new & PAD_START) {
			//PlaySnd(1);
			#ifdef SFX
			MultiMusic_Play(STA_SFX);			
			delay_s = 0x40;
			while (delay_s>0) {ppu_wait_nmi(); delay_s--;}
			#endif
			while (pad1 & PAD_START) {pad1 = pad_poll(0);}
			RunGame(menu_offset);
		}

		if (pad1 & PAD_RIGHT) {
			PlaySnd(0);
			#ifdef SFX
			MultiMusic_Play(SEL_SFX);
			ppu_wait_nmi();
			#endif
			//page++;			
			//menu_offset = (menu_offset + MAXLINES) % (cart_inserted ? CART2_GAMES : CART1_GAMES);
			/*
			if (menu_offset >= (cart_inserted ? CART2_GAMES : CART1_GAMES)) {
				menu_offset = (cart_inserted ? CART2_GAMES : CART1_GAMES)-1;
			}
			if (menu_offset >= (cart_inserted ? CART2_GAMES : CART1_GAMES)) {
				page = 0;
				menu_offset = 0;
			}
			*/
			menu_offset += MAXLINES;

			if (menu_offset >= MAXGAME) {
				if (page >= MAXGAMES-1) {
					menu_offset = (menu_offset % MAXLINES);
				} else {
					menu_offset = MAXGAME-1;
				}
			}			

			page = (menu_offset/MAXLINES);
			DrawPage(page, 0);
			m_d = menu_offset % MAXLINES;
		}

		if (pad1 & PAD_LEFT) {		
			PlaySnd(0);				
			#ifdef SFX
			MultiMusic_Play(SEL_SFX);
			ppu_wait_nmi();
			#endif
			/*
			if (page <= 0) {
				page = cart_inserted ? (CART2_GAMES/MAXLINES) : (CART1_GAMES/MAXLINES);
			} else {
				page--;			
			}
			menu_offset -= MAXLINES;
			if (menu_offset < 0) {
				menu_offset = (cart_inserted ? CART2_GAMES : CART1_GAMES)-1;
			}
			*/
			if (page <= 0) {	
				menu_offset = (menu_offset % MAXLINES) + (MAXLINES * (MAXGAMES-1));
				if (menu_offset >= MAXGAME) {
					menu_offset = MAXGAME-1;
				}
			} else {
				menu_offset = (menu_offset - MAXLINES) % MAXGAME;
			}
			page = (menu_offset/MAXLINES);

			DrawPage(page, 0);
			m_d = menu_offset % MAXLINES;
		}

		if (pad1_new & PAD_DOWN) {
			menu_hold_delay = 0;
		}

		if ((pad1 & PAD_DOWN) && menu_hold_delay <= 0) {
			PlaySnd(0);
			#ifdef SFX
			MultiMusic_Play(SEL_SFX);
			ppu_wait_nmi();
			#endif			
			menu_offset++;
			m_d++;
			if (menu_offset >= MAXGAME) {
				if (page > 0) {
					m_d = MAXLINES;
				} else {
					m_d = 0;
				}
				menu_offset = 0;
			}
			if (m_d >= MAXLINES) {
				m_d = 0;
				page = (menu_offset/MAXLINES);
				DrawPage(page, 0);
			}			
			menu_hold_delay = 8;
		}

		if (pad1_new & PAD_UP) {
			menu_hold_delay = 0;
		}

		if ((pad1 & PAD_UP) && menu_hold_delay <= 0) {
			PlaySnd(0);
			#ifdef SFX
			MultiMusic_Play(SEL_SFX);
			ppu_wait_nmi();
			#endif			
			if (menu_offset <= 0) {
				menu_offset = MAXGAME-1;
				
				m_d = m_d = (menu_offset % MAXLINES);
				page = (menu_offset/MAXLINES);

				if (page > 0) DrawPage(page, 0);				
			} else {
				menu_offset--;
				if (m_d <= 0) {
					m_d = (menu_offset % MAXLINES);
					page = (menu_offset/MAXLINES);
					DrawPage(page, 0);
				} else {
					m_d--;
				}
			}		
			menu_hold_delay = 8;
		}		

		if (menu_hold_delay) menu_hold_delay--;
	}
}

void DrawText(unsigned char *text, unsigned char length, int address, unsigned char fast) {
	memfill(text_tmp, 0, sizeof(text_tmp));

	for (text_offs = 0; text_offs<length; text_offs++) {
		if (text[text_offs] >= 0x30 && text[text_offs] <= 0x39) {
			text_tmp[text_offs] = 2*(text[text_offs]-0x30)+0x20+CHARA_OFFSET;
		} else if (text[text_offs] >= 0x00 && text[text_offs] <= 0x1f) {
			text_tmp[text_offs] = 0x00+CHARA_OFFSET;
		} else if (text[text_offs] >= 0x20 && text[text_offs] <= 0x2f) {
			text_tmp[text_offs] = 2*(text[text_offs]-0x20)+CHARA_OFFSET;
		} else {
			text_tmp[text_offs] = 2*(text[text_offs]-0x40)+0x40+CHARA_OFFSET;
		}		
	}

	if (fast) {
		vram_adr(address);
		vram_write(text_tmp, length);
	} else {
		set_vram_buffer();
		multi_vram_buffer_horz(text_tmp, length, address);
	}

	if (!fast) ppu_wait_nmi();

	for (text_offs = 0; text_offs<length; text_offs++) {
		if (text[text_offs] >= 0x30 && text[text_offs] <= 0x39) {
			text_tmp[text_offs] = 2*(text[text_offs]-0x30)+0x20+1+CHARA_OFFSET;
		} else if (text[text_offs] >= 0x00 && text[text_offs] <= 0x1f) {
			text_tmp[text_offs] = 0x00+CHARA_OFFSET;
		} else if (text[text_offs] >= 0x20 && text[text_offs] <= 0x2f) {
			text_tmp[text_offs] = 2*(text[text_offs]-0x20)+1+CHARA_OFFSET;
		} else {
			text_tmp[text_offs] = 2*(text[text_offs]-0x40)+0x40+1+CHARA_OFFSET;
		}		
	}

	if (fast) {
		vram_adr(address+0x20);
		vram_write(text_tmp, length);
	} else {
		set_vram_buffer();
		multi_vram_buffer_horz(text_tmp, length, address+0x20);
	}
}

void DrawPage(unsigned char page, unsigned char fast) {		
	if (!fast) {
    	set_vram_buffer();    

		oam_clear();
		//oam_spr(M_X+(3*ar_pos), M_Y+(16*(menu_offset%MAXLINES)), 0x34, 0x0);
		//oam_spr((M_X+8)+(3*ar_pos), M_Y+(16*(menu_offset%MAXLINES)), 0x35, 0x0);
		oam_spr(M_X, M_Y+(16*(menu_offset%MAXLINES)), ARR_OFFSET, 0x0);
		oam_spr(M_X+8, M_Y+(16*(menu_offset%MAXLINES)), ARR_OFFSET+1, 0x0);
	}    

    //multi_vram_buffer_horz("110\0GAMES", 9, NTADR_A(11,3));         

    for (line = 0; line<MAXLINES; line++) {
		memfill(text_tmp_i, 0x0, sizeof(text_tmp_i));

		tmp_digit = (MAXLINES*page)+line+1;		
		dig_offset = 2;
		//p_offset = 0;
        if (tmp_digit > MAXGAME) {
            text_tmp_i[0] = 0x00;
            text_tmp_i[1] = 0x00;
            text_tmp_i[2] = 0x00;
            text_tmp_i[3] = 0x00;
        } else {			
            digits[3] = 0x2e;            			
            digits[2] = (tmp_digit % 10) + 0x30;
            tmp_digit /= 10;			
			digits[1] = (tmp_digit % 10) + 0x30;  
			if (((MAXLINES*page)+line+1) >= 10) {
				digits[1] = (tmp_digit % 10) + 0x30;            
			} else {
				dig_offset--;
				digits[1] = 0x00;
			}
            tmp_digit /= 10;
			digits[0] = (tmp_digit % 10) + 0x30;
            if (((MAXLINES*page)+line+1) >= 100) {
				digits[0] = (tmp_digit % 10) + 0x30;
			} else {
				dig_offset--;
				digits[0] = 0x00;
			}
        }


		memcpy(text_tmp_i+dig_offset, digits, 4);
		//DrawText(digits, 6, NTADR_A(3+dig_offset,6+(2*line)), fast);
        //ppu_wait_nmi();
		if (tmp_digit > MAXGAME) {
			memfill(text_tmp_i, 0x00, sizeof(text_tmp_i));
			//#DrawText(blank, 16, NTADR_A(8+dig_offset,6+(2*line)), fast);
		} else {
			rom_name_offset = (16*(line+rom_shift)) + (16*MAXLINES)*page;
			set_prg_8000(rom_name_offset >= 0x2000 ? ROM_NAMES+1 : ROM_NAMES);
			memcpy(text_tmp_i+(dig_offset+4), &PEEK(0x8000 + (rom_name_offset % 0x2000)), 16);
			//memcpy(text_tmp_i+(dig_offset+4), "AAAAAAAAAAAAAAAA", 16);
			//DrawText(&PEEK(0x8000+(16*line)+((16*MAXLINES)*page)), 16, NTADR_A(8+dig_offset,6+(2*line)), fast);
			//DrawText(text_tmp_i, sizeof(text_tmp_i), NTADR_A(3,6+(2*line)), fast);
		}

		DrawText(text_tmp_i+2, TEXT_LENGTH, NTADR_A(8,TEXT_OFFSET+(2*line)), fast);
		if (!fast) ppu_wait_nmi();
    }
    
    //ppu_dma_render();
    /*
    POKE(0x700, 0x1);
    POKE(0x701, 0x2);
    POKE(0x702, 0x4);
    POKE(0x703, 0x8);

    vram_adr(NTADR_A(0,0));

    POKE(0x4034, 0x0);
    POKE(0x4014, 0x7);        
    */
}

void TestMode() {
	pal_bg(MenuPal);
	pal_spr(MenuPal_S);
	vram_fill(0x80, 0x3c0);
	render_title();
	DrawText("ROM TEST", 8, NTADR_A(10,9), 1);
	ppu_on_all();
	checking_sram = 1;
	POKE(0x6003, 0x80);
	set_wram_mode(WRAM_ON);
	set_rand(PEEK(0x6000)+PEEK(0x6001)+1);
	while (1) {
		if (checking_sram) {			
			sram_expected = rand8();
			POKE(0x6000+sram_offset, sram_expected);
			if (PEEK(0x6000+sram_offset) != sram_expected) {
				checking_sram = 0;				
				DrawText("SRAM FAIL", 9, NTADR_A(10,12), 0);				
				set_wram_mode(WRAM_OFF);
			}
			sram_offset++;
			if (sram_offset >= 0x2000) {
				checking_sram = 0;
				DrawText("SRAM PASS", 9, NTADR_A(10,12), 0);		
				set_wram_mode(WRAM_OFF);		
			}
		} else {
			ppu_wait_nmi();
		}		
	}
	//RunGame(TESTROM);
}