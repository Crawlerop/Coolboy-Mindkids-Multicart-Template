import sys

if __name__ == "__main__":
    files = [open(sys.argv[1]), open(sys.argv[2])]
    #out = ""
    bank_id = 0
    for f in files:
        print(f"; BANK {bank_id+1}")
        for fl in f:            
            if fl.startswith("; 412C_"):                
                break
            print(fl, end="")
        bank_id += 1

    print("; 412C_SPEC1")
    bank_id = 0
    for f in files:
        #print(f"; BANK {bank_id+1}")
        for fl in f:            
            if fl.startswith("; 412C_"):                
                break
            if (bank_id == 0 or fl.startswith("   dw")): print(fl, end="")
        bank_id += 1

    print("; 412C_SPEC2")
    bank_id = 0
    for f in files:
        #print(f"; BANK {bank_id+1}")
        for fl in f:            
            if fl.startswith("; 412C_"):                                
                break
            if (bank_id == 0 or not fl.startswith("Games_Start")): print(fl, end="")
        bank_id += 1        
    print("; 412C_SPEC3")
    print(next(files[0]))