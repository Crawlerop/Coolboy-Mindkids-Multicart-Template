import glob
import logging
import sys
import os

bank_sizes = [0x40000, 0x80000]


NEXT_BANK = 0
MAX_BANK = 64

BANK_SHIFT_OFFS = 0x0
SORT_MODE = 1

#TEROM_LAST_BANK = False

def sort_based_on_num(data: dict):
    return {k: v for k,v in sorted(data.items(), key=lambda item: item[1])}

def sort_based_on_num_b(data: dict):
    return {k: v for k,v in reversed(sorted(data.items(), key=lambda item: item[1]))}


def mask_dict(data: dict, k_mask):
    out = {}
    for k in data:
        if k != k_mask:
            out[k] = data[k]
    return out

def padi(g):
    return (" "*(3-len(g)))+g

if __name__ == "__main__":
    bank_id = 0
    space_used = 0

    out_data_r = ".segment \"ROM0\"\n"
    out_data_t = ".segment \"ROM_OFFS\"\n"

    out_data_l = ".segment \"MENU_GAME_DATA_1\"\n"
    out_data_c = ""

    c_table = []
    left_space = {}

    rom_prg_padding = {}
    rom_prg_sizes = {}
    rom_o_prg_sizes = {}
    rom_chr_sizes = {}
    rom_chr_ram = {}

    rom_mirr = {} 
    rom_mapp = {} 
    rom_sys = {}
    game_id = 0

    tot_games = []
    gf_id = []
    gf_shf = []

    dirs = next(os.walk(f"{sys.argv[1]}"))[1]
    #tot_games.append(f"{sys.argv[1]}\TEST.nes")

    for d in dirs:
        g = glob.glob(f"{sys.argv[1]}\{d}\*.nes")
        gf_shf.append(len(tot_games))
        tot_games += g
        gf_id.append(len(g))
        #out_data_c += f"const unsigned int CAT_{d.upper()}_GAMES = {len(g)};\n"
        #out_data_c += f"const unsigned char CAT_{d.upper()}_TEXT[5] = \"{padi(str(len(g)))}-1\";\n"
        out_data_c += f"const unsigned char CAT_{d.upper()}_TEXT[8] = \"{padi(str(len(g)))} IN 1\";\n"
        c_table.append(d.upper())

    out_data_c += "const unsigned int CAT_NO[] = {\n"+", ".join([str(x) for x in gf_id])+"\n};\n"
    out_data_c += "const unsigned int CAT_SHF[] = {\n"+", ".join([str(x) for x in gf_shf])+"\n};\n"
    out_data_c += "const unsigned char * const CAT_TXT[] = {\nCAT_"+"_TEXT, CAT_".join(c_table)+"_TEXT\n};\n"
    out_data_c += f"#define ROMS_MAX {len(tot_games)}"                
        
    #print(tot_games)
    #rom_offs = 0
    #tot_games = 
    for f in tot_games:
        fd = open(f, "rb")
        assert fd.read(4) == b"NES\x1a", f
        prg_size = fd.read(1)[0]*16384
        chr_size = fd.read(1)[0]*8192
        mirr_mapp = fd.read(1)[0]
        mapp_2 = fd.read(1)[0]
        mapp_3 = fd.read(1)[0]
        fd.read(4)
        mapp_sys = fd.read(1)[0]        


        mirr = mirr_mapp & 1
        mapp = ((mapp_3 & 0xf) << 8) | ((mapp_2 >> 4) << 4) | (mirr_mapp >> 4)
        #print(mapp)

        #out_counter += prg_size*16
        #out_data += f"Game"

        #out_counter += chr_size*16

        #game_id += 1
        rom_prg_sizes[game_id] = prg_size
        rom_o_prg_sizes[game_id] = prg_size
        rom_chr_sizes[game_id] = chr_size       
        rom_mirr[game_id] = mirr
        rom_mapp[game_id] = mapp
        rom_chr_ram[game_id] = int(chr_size <= 0)
        rom_sys[game_id] = mapp_sys & 0xf
        if rom_prg_sizes[game_id] < 0x20000 and rom_mapp[game_id] not in [0,3,185]:
            rom_prg_padding[game_id] = (0x20000-rom_prg_sizes[game_id])//rom_prg_sizes[game_id]
            rom_prg_sizes[game_id] = 0x20000            
    
        '''
        t_rom_size += prg_size+chr_size

        rom_prg[game_id] = prg_size
        rom_chr[game_id] = chr_size
        rom_mirr[game_id] = mirr
        rom_mapper[game_id] = mapp
        chr_ram_flag[game_id] = int(chr_size == 0)
        files.append(f)
        '''

        game_id += 1
    
    p_id = 0
    c_id = 0
    
    rom_mode = 1
    loaded_p = False
    loaded_c = False

    rom_tmp = {}    
    rom_tmp_sz = {}

    rom_p_sorted = []
    rom_c_sorted = []

    area_offset = 0
    bank_limit = bank_sizes[0]

    chr_left = prg_left = len(tot_games)

    game_offset = 0
    #sz_te_sizes = []

    def switch_bank(flush: bool=False):
        global rom_mode, bank_id, out_data_r, bank_limit, rom_tmp, rom_tmp_sz, area_offset, game_offset, space_used
        if not flush:
            if rom_mode == 1 and prg_left > 0:
                rom_mode = 0
            elif rom_mode == 0 and chr_left > 0:
                rom_mode = 1
            
            bank_id += 1
            if (bank_id < len(bank_sizes)):
                bank_limit = bank_sizes[bank_id]
            else:
                bank_limit = bank_sizes[len(bank_sizes)-1]

            
        
        #print(sort_based_on_num_b(rom_tmp_sz))
        rom_index = [x for x in sort_based_on_num_b(rom_tmp_sz).keys()]
        for x in rom_index:
            out_data_r += rom_tmp[x]     

        if not flush:   
            rom_tmp_sz = {}
            rom_tmp = {}
            
            game_offset += 0x80000
            #print(bank_id)
            if bank_id == NEXT_BANK: 
                game_offset += BANK_SHIFT_OFFS                
                out_data_r += f"; BANK 2\n"
            elif bank_id == MAX_BANK:
                print("WARNING: ROM Size Overflow", file=sys.stderr)
            space_used = game_offset
            area_offset = 0
            out_data_r += f".segment \"ROM{bank_id}\"\n"            

    if SORT_MODE == 1:
        #rom_p_sorted.append(0)
        #rom_c_sorted.append(0)
        
        #print(mask_dict(rom_prg_sizes, 400))
        #input()

        #print(sort_based_on_num(mask_dict(rom_prg_sizes, 0)))
        #rom_p_sorted.extend([x for x in sort_based_on_num_b(mask_dict(rom_prg_sizes, 0)).keys()])
        #rom_c_sorted.extend([x for x in sort_based_on_num_b(mask_dict(rom_chr_sizes, 0)).keys()]) 
        rom_p_sorted = [x for x in sort_based_on_num_b(rom_prg_sizes).keys()]
        rom_c_sorted = [x for x in sort_based_on_num_b(rom_chr_sizes).keys()]
        print(len(rom_p_sorted))
        print(len(rom_c_sorted))

        #print(rom_prg_sizes[rom_p_sorted[0]])

    while True:
        if chr_left <= 0 and prg_left <= 0: break
        
        if rom_mode == 0: 
            if SORT_MODE == 1 and not loaded_p: 
                loaded_p = True                
                p_id = rom_p_sorted.pop(0)
                #print(rom_prg_sizes[p_id])

            if (area_offset+rom_prg_sizes[p_id]) > bank_limit:
                left_space[bank_id] = hex(bank_limit-area_offset)
                switch_bank()
            else:               
                if SORT_MODE == 1: loaded_p = False       
                rom_tmp[p_id] = ""
                if (rom_chr_sizes[p_id]) <= 0: rom_tmp[p_id] += f"Game{p_id+1:04d}_c:\nGame{p_id+1:04d}_c_e:\n"
                prg_line = f".incbin \"{tot_games[p_id]}\",$10,${hex(rom_o_prg_sizes[p_id])[2:]}\n"
                if p_id in rom_prg_padding:
                    prg_line += (prg_line)*rom_prg_padding[p_id]
                    #print(prg_line)
                    #input()
                rom_tmp[p_id] += f"Game{p_id+1:04d}:\n{prg_line}Game{p_id+1:04d}_e:\n"
                rom_tmp_sz[p_id] = rom_prg_sizes[p_id]
                area_offset += rom_prg_sizes[p_id]
                space_used += rom_prg_sizes[p_id]                
                if SORT_MODE == 0: p_id += 1
                prg_left -= 1            
                if (prg_left <= 0 and chr_left > 0):
                    switch_bank()
                pass

        elif rom_mode == 1:
            if SORT_MODE == 1 and not loaded_c: 
                loaded_c = True
                c_id = rom_c_sorted.pop(0)

            if (area_offset+rom_chr_sizes[c_id]) > bank_limit:
                left_space[bank_id] = hex(bank_limit-area_offset)
                switch_bank()
            else:
                if SORT_MODE == 1: loaded_c = False            
                rom_tmp[c_id] = ""
                if rom_chr_sizes[c_id] > 0: rom_tmp[c_id] = f"Game{c_id+1:04d}_c:\n.incbin \"{tot_games[c_id]}\",${hex(rom_o_prg_sizes[c_id]+0x10)[2:]},${hex(rom_chr_sizes[c_id])[2:]}\nGame{c_id+1:04d}_c_e:\n"
                rom_tmp_sz[c_id] = rom_chr_sizes[c_id]
                area_offset += rom_chr_sizes[c_id]
                space_used += rom_chr_sizes[c_id]
                if SORT_MODE == 0: c_id += 1
                chr_left -= 1
                if (chr_left <= 0 and prg_left > 0):
                    switch_bank()
                pass

        if area_offset >= bank_limit:
            switch_bank()

    switch_bank(True)

    s_offs = 0

    for x in range(len(tot_games)):
        out_data_t += f'dw Game_{x+1:04d}\n'
        out_data_l += f'Game_{x+1:04d}:\n'
        out_data_l += f'db {rom_mirr[x]^1}\n'
        #nrom_check = rom_chr_sizes[x] <= 8192 and rom_prg_sizes[x] <= 32768
        nrom_check = 0
        bpp_check = int(rom_sys[x] in [7,8,9,0xa])        
        
        t_mapper = 0 # 0 = MMC3, 1 = MMC1, 2 = UNROM, 3 = CNROM, 4 = NROM
        if rom_mapp[x] in [0,3,185]:
            t_mapper = 1
        #elif rom_mapp[x] == 1:
        #    t_mapper = 1
        #elif rom_mapp[x] in [2,71]:
        #    t_mapper = 2
        elif rom_mapp[x] not in [4,206]:            
            print(f"{tot_games[x]} has unknown mapper: {rom_mapp[x]}", file=sys.stderr)
        
        #if rom_mapp[x] in [256,296]:
        #    print(f"{tot_games[x]}", file=sys.stderr)

        out_data_l += f'gametable Game{x+1:04d},Game{x+1:04d}_e,Game{x+1:04d}_c,Game{x+1:04d}_c_e,{nrom_check},{t_mapper}\n'
        out_data_l += f'dw LoadSub\n'

        out_data_l += f'db ${hex((t_mapper << 4) | rom_chr_ram[x])[2:]}\n'
        #out_data_l += f'bind_vrom Game{x+1:04d}_c,{bpp_check}\n\n'
        out_data_l += f'\n'

        s_offs += 1
        if s_offs >= 819:
            s_offs = 0
            out_data_l += ".segment \"MENU_GAME_DATA_2\"\n"

    open("games.h", "w").write(out_data_c)
    open("games.inc", "w").write(out_data_r+"\n"+out_data_t+"\n"+out_data_l)
    open("game_names.txt", "w").write("\n".join([os.path.splitext(os.path.split(x)[1])[0].upper().replace("__"," ").replace("_","") for x in tot_games]))
    print(f"Wasted space: {left_space}", file=sys.stderr)
    print(f"ROM space used: {space_used}, h: {hex(space_used)}, mb: {round(space_used/0x100000, 4)}, space left: {(MAX_BANK*0x80000)-space_used}, {hex((MAX_BANK*0x80000)-space_used)}, mb: {round(((MAX_BANK*0x80000)-space_used)/0x100000, 4)}, banks: {bank_id+1}, offset: {game_offset}, offset_h: {hex(game_offset)}", file=sys.stderr)
    print(f"Total Games: {len(tot_games)}", file=sys.stderr)
    #print(out_data_r)
    #print(out_data_t)
    #print(out_data_l)