import sys

def pad(a):
    return "0"*(2-len(a)) + a 

if __name__ == "__main__":
    data = open(sys.argv[1], "rb").read(0x30)
    offset = 0

    out_upper = []
    out_lower = []

    while offset<len(data):
        out = (data[offset] >> 4) << 8 | (data[offset+1] >> 4) << 4 | (data[offset+2] >> 4)  
        out_upper.append(out >> 6)
        out_lower.append(out & 0x3f)
        offset += 3

    print("_palette_lower:")
    print("    .export _palette_lower")
    print(f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_lower)}")
    for _ in range(3):
        print(f"   .byte $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF")

    print("_palette_upper:")
    print("    .export _palette_upper")
    print(f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_upper)}")
    for _ in range(3):
        print(f"   .byte $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF")