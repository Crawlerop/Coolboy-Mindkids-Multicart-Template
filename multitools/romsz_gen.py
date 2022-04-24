import sys
if __name__ == "__main__":
    ROM_SIZE = int(sys.argv[1])
    ROM_ID = int(sys.argv[2])
    ROM_OFFSET = int(sys.argv[3], 16)
    s = 0x80000+ROM_OFFSET
    for r in range((ROM_SIZE//512)-1):
        print(f"ROM{r+1+ROM_ID}: start = ${hex(s)[2:]}, size = $80000, file = %O, fill = yes;")
        s += 0x80000
        if (s == 0x1000000):
            print("\n# 2nd ROM bank\n")
        elif (s == 0x2000000):
            print("\n# 3rd ROM bank\n")
        elif (s == 0x3000000):
            print("\n# 4th ROM bank\n")            

    print()

    for r in range((ROM_SIZE//512)-1):
        print(f"ROM{r+1+ROM_ID}:    load = ROM{r+1+ROM_ID},           type = ro, define = yes;")