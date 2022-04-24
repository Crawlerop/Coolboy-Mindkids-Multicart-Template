import glob
import logging
import sys

def sort_based_on_num(data: dict):
    return {k: v for k,v in sorted(data.items(), key=lambda item: item[1])}

#Games_Cust_Sub = {0: "Bros2_Sub", 33: "ClayShoot_Sub", 34: "Olympics_Sub", 69: "Gradius_Sub", 97: "GameSuper_Sub", 98: "SUGradius_Sub", 99: "GameSuper_Sub", 100: "SUBattle_Sub_2", 101: "GameSuper_Sub", 102: "SUGalaxian_Sub", 103: "GameSuper_Sub", 104: "SUPacman_Sub", 105: "GameSuper_Sub", 106: "SUXevious_Sub_2", 107: "GameSuper_Sub", 108: "Bros2_Sub", 109: "FastBros2_Sub"}
#Inverted_Mirr = [108]
#Games_Repeat = {33: 30, 97: 17, 98: 68, 99: 6, 100: 6, 101: 19, 102: 4, 103: 65, 104: 9, 105: 20, 106: 76, 107: 15, 108: 0, 109: 0}
Games_Cust_Sub = {}
Inverted_Mirr = []
Games_Repeat = {}
CHR_Invert_Fix = []
FORCE_NROM = []
NROM_DETECT = True

#ROM_MAX_SIZE = 15872
ROM_MAX_SIZE = 512*30
BANKSWITCH_SZ = 31

if __name__ == "__main__":
    assert not (ROM_MAX_SIZE % 512)
    FORCE_NROM = [int(x) for x in sys.argv[6].split(",")]
    
    print(f"MultiCreator:\nMax ROM size: {hex(ROM_MAX_SIZE)}\n", file=sys.stderr)
    out_data = ""
    out_counter = 0
    tmp_buf = 0

    out_rom_bank = 0

    starting_bank = int(sys.argv[1])
    starting_id = int(sys.argv[2])
    file_start_offset = int(sys.argv[4])
    multi_id = int(sys.argv[5])

    t_rom_size = 0

    rom_prg = {}
    rom_chr = {}
    rom_mirr = {}
    rom_mapper = {}
    chr_ram_flag = {}



    files = []

    game_id = 0
    g_off = 0

    out_data += f'.segment "ROM{starting_bank+1}"\n'
    #print(out_data)
    #out_table +=

    for f in glob.glob(f"{sys.argv[3]}/*.nes")[file_start_offset:]:
        fd = open(f, "rb")
        assert fd.read(4) == b"NES\x1a", f
        prg_size = fd.read(1)[0]*16
        chr_size = fd.read(1)[0]*8
        mirr_mapp = fd.read(1)[0]
        mapp_2 = fd.read(1)[0]

        mirr = mirr_mapp & 1
        mapp = ((mapp_2 >> 4) << 4) | (mirr_mapp >> 4)

        #out_counter += prg_size*16
        #out_data += f"Game"

        #out_counter += chr_size*16

        #game_id += 1
        
        if (t_rom_size+(prg_size+chr_size)) > ROM_MAX_SIZE:
            logging.warning(f"Game id: {(game_id+1)+starting_id}, Maximum rom size exceeded: {t_rom_size+(prg_size+chr_size)} > {ROM_MAX_SIZE}, {t_rom_size}, in: {f}")            
            logging.warning(len(rom_prg))
            logging.warning(len(rom_chr))
            logging.warning(f"To continue, run {sys.argv[0]} {BANKSWITCH_SZ*(starting_bank+1)} {(game_id+starting_id)} {sys.argv[3]} {(game_id+starting_id)}")
            break        

        t_rom_size += prg_size+chr_size

        rom_prg[game_id] = prg_size
        rom_chr[game_id] = chr_size
        rom_mirr[game_id] = mirr
        rom_mapper[game_id] = mapp
        chr_ram_flag[game_id] = int(chr_size == 0)
        files.append(f)
    
        game_id += 1

    logging.warning(f"{t_rom_size} out of {ROM_MAX_SIZE} were used. {ROM_MAX_SIZE-t_rom_size} remaining")

    rom_prg_s = sort_based_on_num(rom_prg)
    rom_chr_s = sort_based_on_num(rom_chr)

    rom_prg_remain = list(rom_prg_s.keys())
    rom_chr_remain = list(rom_chr_s.keys())

    num_games = len(rom_prg_s.keys())

    def overflow_flip():
        global out_counter, out_rom_bank, out_data
        out_counter = 0
        out_rom_bank += 1
        out_data += f'.segment "ROM{out_rom_bank+1+starting_bank}"\n'

    def flip():
        global out_rom_bank, out_data        
        out_rom_bank += 1
        out_data += f'.segment "ROM{out_rom_bank+1+starting_bank}"\n'        

    def check_overflow():
        global out_counter
        #print(out_counter)
        assert out_counter <= 512, out_counter        
        if (out_counter == 512):
            overflow_flip()        

    while True:
        if len(rom_chr_remain) <= 0 and len(rom_prg_remain) <= 0: break
        tmp_buf = 0
        if len(rom_prg_remain) > 0:
            while True:
                if tmp_buf >= 512 or len(rom_prg_remain) <= 0: 
                    assert tmp_buf <= 512, tmp_buf
                    if len(rom_prg_remain) <= 0: out_data += "; END PRG\n"
                    if len(rom_chr_remain) > 0 or len(rom_prg_remain) > 0: flip()
                    break
                rom_prg_tmp = rom_prg_remain.pop()
                tmp_buf += rom_prg_s[rom_prg_tmp]
                out_data += f'Game{rom_prg_tmp+1+starting_id+(1000*multi_id):04d}_p:\n'        
                out_data += f'.incbin "{files[rom_prg_tmp]}", $10, ${hex(rom_prg_s[rom_prg_tmp]*0x400)[2:]} ; {rom_prg_s[rom_prg_tmp]}k\n'
                out_data += f'Game{rom_prg_tmp+1+starting_id+(1000*multi_id):04d}__p:\n'    
                #print(tmp_buf, rom_prg_s[rom_prg_tmp])    
        
        tmp_buf = 0
        if len(rom_chr_remain) > 0:
            while True:
                if tmp_buf >= 512 or len(rom_chr_remain) <= 0: 
                    assert tmp_buf <= 512, tmp_buf
                    if len(rom_chr_remain) <= 0: out_data += "; END CHR\n"
                    if len(rom_chr_remain) > 0 or len(rom_prg_remain) > 0: flip()
                    break                
                rom_chr_tmp = rom_chr_remain.pop()
                tmp_buf += rom_chr_s[rom_chr_tmp]
                out_data += f'Game{rom_chr_tmp+1+starting_id+(1000*multi_id):04d}_c:\n'                        
                out_data += f'.incbin "{files[rom_chr_tmp]}", ${hex((rom_prg_s[rom_chr_tmp]*0x400)+0x10)[2:]}, ${hex(rom_chr_s[rom_chr_tmp]*0x400)[2:]} ; {rom_chr_s[rom_chr_tmp]}k\n'
                out_data += f'Game{rom_chr_tmp+1+starting_id+(1000*multi_id):04d}__c:\n'        

    out_data += "; 412C_SPEC1\n"
    '''
    for r in rom_prg_s.keys():
        out_data += f'Game{r+1:04d}_p:\n'
        out_data += f'.incbin "{files[r]}", $10, ${hex(rom_prg_s[r]*0x400)[2:]} ; {rom_prg_s[r]}k\n'
        out_data += f'Game{r+1:04d}__p:\n'

        if (out_counter + rom_prg_s[r] > 512):
            overflow_flip()
        
        out_counter += rom_prg_s[r]
        check_overflow()
    
    overflow_flip()
    #check_overflow()

    for r in rom_chr_s.keys():        
        out_data += f'Game{r+1:04d}_c:\n'        
        if r in CHR_Invert_Fix and rom_chr_s[r] == 8:
            out_data += f'.incbin "{files[r]}", ${hex((rom_prg_s[r]*0x400)+0x10+0x1000)[2:]}, $1000 ; {rom_chr_s[r]//2}k\n'
            out_data += f'.incbin "{files[r]}", ${hex((rom_prg_s[r]*0x400)+0x10)[2:]}, $1000 ; {rom_chr_s[r]//2}k\n'
        else:
            out_data += f'.incbin "{files[r]}", ${hex((rom_prg_s[r]*0x400)+0x10)[2:]}, ${hex(rom_chr_s[r]*0x400)[2:]} ; {rom_chr_s[r]}k\n'
        out_data += f'Game{r+1:04d}__c:\n'

        if (out_counter + rom_chr_s[r] > 512):
            overflow_flip()

        out_counter += rom_chr_s[r]
        check_overflow()

    #check_overflow()
    '''

    num_games_r = len(rom_prg_s.keys())+len(Games_Repeat.keys())

    out_data += f'.segment "MENU_GAME_DATA_{multi_id+1}"\n_GameList_{(1000*multi_id):04d}:\n'
    for r in range(num_games_r):
        out_data += f'   dw Game{r+1+starting_id+(1000*multi_id):04d}\n'
    out_data += "; 412C_SPEC2\n"
    out_data += f'Games_Start_{(1000*multi_id):04d}:\n'
    for r in range(num_games_r):     
        s = r-g_off
        #print(s)
        #p = r

        if r in Games_Repeat:
            s = Games_Repeat[r]
            g_off += 1
            #print(g_off)
            #p = Games_Repeat[r]

        out_data += f'Game{r+1+starting_id+(1000*multi_id):04d}:\n'
        if r in Inverted_Mirr:
            out_data += f'   db ${hex(rom_mirr[s])[2:]}\n'
        else:
            out_data += f'   db ${hex(rom_mirr[s]^1)[2:]}\n'        
        out_data += f'   gametable Game{s+1+starting_id+(1000*multi_id):04d}_p,Game{s+1+starting_id+(1000*multi_id):04d}__p,Game{s+1+starting_id+(1000*multi_id):04d}_c,Game{s+1+starting_id+(1000*multi_id):04d}__c,{int(NROM_DETECT and ((rom_chr_s[r] == 8 and rom_prg_s[r] <= 32) or r in FORCE_NROM))}\n'
        if r in Games_Cust_Sub:
            out_data += f'   dw {Games_Cust_Sub[r]}\n'
        else:
            out_data += f'   dw LoadSub\n'

        t_mapper = 0 # 0 = MMC3, 1 = MMC1, 2 = UNROM, 3 = CNROM, 4 = NROM
        #if rom_mapper[s] in [0,185]:
        #    t_mapper = 4
        #elif rom_mapper[s] == 3:
        if rom_mapper[s] in [0,3]:
            t_mapper = 3         
        elif rom_mapper[s] == 1:
            t_mapper = 1
        elif rom_mapper[s] in [2,71]:
            t_mapper = 2
        elif rom_mapper[s] not in [4,206,256,296,185]:
            logging.warning(f"Game {r+starting_id+1} has incompatible mapper: {rom_mapper[s]}")        

        if rom_mapper[s] in [256,296]:
            logging.warning(f"Onebus Game: {r+starting_id+1}")

        if t_mapper not in [0,3,4]:
            logging.warning(f"Game {r+starting_id+1} were only compatible with VT32 because this game uses mapper {t_mapper}")

        if t_mapper == 3 and rom_chr_s[r] > 8:
            logging.warning(f"Game {r+starting_id+1} uses CNROM")

        if rom_chr_s[r] == 0:
            logging.warning(f"Game {r+starting_id+1} uses CHR-RAM, which is only supported in 412c and VT32 mappers")

        out_data += f'   db ${hex((t_mapper << 4) | chr_ram_flag[s])[2:]}\n'
    out_data += "; 412C_SPEC3\n"
    out_data += f'Games_End_{(1000*multi_id):04d}:\n'
    out_data += open("footer.inc", "r").read()

    print(f"Banks used: {out_rom_bank+1}",file=sys.stderr)
    print(out_data)
    #print(rom_prg_s, rom_chr_s, num_games)
    #open(sys.argv[3], "w").write(out_data)