import sys
from PIL import Image
import io

def pad(a):    
    return "0"*(2-len(a)) + a 

#def pad_bytes(data):
#    return b"\0"*(2)

def generate_palette_t(data):
    offset = 0

    out_upper = []
    out_lower = []
    
    out_c = ""

    out_txt = ""

    while offset<len(data):
        out = (data[offset+2] >> 4) << 8 | (data[offset+1] >> 4) << 4 | (data[offset+0] >> 4)  
        out_upper.append(out >> 6)
        out_lower.append(out & 0x3f)
        offset += 3

    out_txt += f"_{sys.argv[6]}_palette_lower:\n"
    out_txt += f"    .export _{sys.argv[6]}_palette_lower\n"
    out_txt += f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_lower[:0x4])}" + ",$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"
    out_txt += f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_lower[0x4:0x8])}" + ",$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"
    out_txt += f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_lower[0x8:0xc])}" + ",$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"
    out_txt += f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_lower[0xc:0x10])}" + ",$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"
    out_c += f"extern const unsigned char {sys.argv[6]}_palette_lower[];\n"
    #for _ in range(3):
    #    out_txt += f"   .byte $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"

    out_txt += f"_{sys.argv[6]}_palette_upper:\n"
    out_txt += f"    .export _{sys.argv[6]}_palette_upper\n"
    out_txt += f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_upper[:0x4])}" + ",$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"
    out_txt += f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_upper[0x4:0x8])}" + ",$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"
    out_txt += f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_upper[0x8:0xc])}" + ",$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"
    out_txt += f"   .byte ${',$'.join(pad(hex(o)[2:]).upper() for o in out_upper[0xc:0x10])}" + ",$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"
    out_c += f"extern const unsigned char {sys.argv[6]}_palette_upper[];\n"

    #for _ in range(3):
    #    out_txt += f"   .byte $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF\n"

    return out_txt, out_c

#def bitstring_to_bytes(s):
#    return int(s, 2).to_bytes((len(s) + 7) // 8, byteorder='big')

def formatTilePlanar(tile, planemap, hflip=False, little=False):
    """Turn a tile into bitplanes.
    Planemap opcodes:
    10 -- bit 1 then bit 0 of each tile
    0,1 -- planar interleaved by rows
    0;1 -- planar interlaved by planes
    0,1;2,3 -- SNES/PCE format
    """
    hflip = 7 if hflip else 0
    if (tile.size != (8, 8)):
        return None
    pixels = list(tile.getdata())
    pixelrows = [pixels[i:i + 8] for i in range(0, 64, 8)]
    if hflip:
        for row in pixelrows:
            row.reverse()
    out = bytearray()

    planemap = [[[int(c) for c in row]
                 for row in plane.split(',')]
                for plane in planemap.split(';')]
    # format: [tile-plane number][plane-within-row number][bit number]

    # we have five (!) nested loops
    # outermost: separate planes
    # within separate planes: pixel rows
    # within pixel rows: row planes
    # within row planes: pixels
    # within pixels: bits
    for plane in planemap:
        for pxrow in pixelrows:
            for rowplane in plane:
                rowbits = 1
                thisrow = bytearray()
                for px in pxrow:
                    for bitnum in rowplane:
                        rowbits = (rowbits << 1) | ((px >> bitnum) & 1)
                        if rowbits >= 0x100:
                            thisrow.append(rowbits & 0xFF)
                            rowbits = 1
                out.extend(thisrow[::-1] if little else thisrow)
    return bytes(out)

def pad_array3(arr, pad=[0,0,0], leng=256):
    assert (not len(arr)%3) and (len(pad) == 3)
    if len(arr)/3 < leng:
        return arr+(pad*(leng-int((len(arr)/3))))
    else:
        return arr    

def generate_palette(palette):
    plt = Image.new("P", (16,16))
    
    pal = pad_array3(palette, [palette[0],palette[1],palette[2]])

    plt.putpalette(pal)
    plt.load()
    return plt

def getPalettes2(img: Image.Image, colors: int=16):
    assert img.mode == "P"
    plt = img.getpalette()[:(colors*3)]
    offset = 0
    outp = []
    while offset<len(plt)-3:
        outp += [plt[offset],plt[offset+1],plt[offset+2]]
        offset += 3
    #print(outp)
    return outp

S_COMPRESS = 1
S_DITHER = 1
S_RESIZE = True
S_RESIZE_SZ = (256,240)

if __name__ == "__main__":
    img_v = Image.open(sys.argv[1])
    max_tiles = int(sys.argv[2])
    tile_start_offset = int(sys.argv[3], 16)
    chr_start_offset = int(sys.argv[4], 16)
    irq_offset = int(sys.argv[5])

    if S_RESIZE:
        img_v = img_v.resize(S_RESIZE_SZ, Image.LANCZOS)

    paluse = None
    if len(sys.argv) > 7:
        paluse = sys.argv[7]

    assert not (max_tiles % 64)

    # Workarounds to use dithering in adaptive palettes
    img_v.load()

    img_q = img_v.quantize(16)

    s_pal = getPalettes2(img_q)    
    s_plt = bytes(img_q.getpalette()[:3*16])

    if paluse:        
        s_plt = bytes([x for x in open(paluse, "rb").read()][:3*16]) 
        s_pal = s_plt

    s_pal_gen = generate_palette_t(s_plt)

    #print(s_pal_gen)

    img_p = generate_palette(s_pal)
    img = img_v._new(img_v.im.convert("P", S_DITHER, img_p.im))
    # End workaround

    assert (not img.width % 8) and (not img.height % 8) # 8x8
        
    y = 0
    px = []
    nt = []

    ch_b = []

    while y<img.height:
        x = 0
        while x<img.width:
            pixel = img.crop((x,y,x+8,y+8))                        
            px_temp = formatTilePlanar(pixel, "0;1;2;3")
            if px_temp not in px:
                px.append(px_temp)
            nt.append(px.index(px_temp))
            x += 8
        y += 8

    lines_c = []        
    nt_data = []        
    irq_data = []

    for o in range(max_tiles//64):
        irq_data.extend([0xf9+o+irq_offset, o+chr_start_offset])
    #irq_data = [0xf9, chr_start_offset+0x00, 0xfa, chr_start_offset+0x01, 0xfb, chr_start_offset+0x02, 0xfc, chr_start_offset+0x03]
    

    c_p = 0+chr_start_offset
    s_l = 0

    lines = 0

    y = 0
    while y<(img.height/8):              
        x = 0
        sofs = len(nt_data)        
        while x<(img.width/8):                
            p_nt = nt[((img.width//8)*y)+x]            
            if (px[p_nt] not in lines_c) or not S_COMPRESS:                                
                lines_c.append(px[p_nt])
            if not S_COMPRESS:                
                ni = len(lines_c)-1                
            else:
                ni = lines_c.index(px[p_nt])
            assert ni >= 0 and ni <= 0xff
            
            nt_data.append(ni+tile_start_offset)            
            if len(lines_c) >= max_tiles:                 
                x = -1
                c_p += max_tiles//64              
                irq_data.append((s_l*8)-1)
                for p in range(max_tiles//64):
                    irq_data.extend([0xf9+p+irq_offset, p+c_p])
                s_l = -1

                nt_data = nt_data[:sofs]
                zz_data = b"".join(lines_c)                
                ch_b.append(zz_data)
                lines_c.clear()
            x += 1
        y += 1        
        s_l += 1

    if len(lines_c) > 0:
        zz_data = b"".join(lines_c)        
        ch_b.append(zz_data)
        lines_c.clear()

    #for x in ch_b:
    #    assert type(x) == bytes, type(x)
        
    irq_data.append(0xff)

    padd = 0x1000

    d = b"".join(ch_b)
    padding = b"\0" * (0 if not (len(d) % padd) else (padd-(len(d) % padd)))
    p#rint(f"Start offset: {hex(chr_start_offset << 11)}", file=sys.stderr)

    print(f"Offset range: {hex(chr_start_offset << 11)}-{hex((chr_start_offset << 11)+len(d+padding))}", file=sys.stderr)
    print(f"Offset range: {hex(chr_start_offset)}-{hex(((chr_start_offset << 11)+len(d+padding)) >> 11)}", file=sys.stderr)


    open(f"{sys.argv[1]}.chr", "wb").write(d + padding)
    open(f"{sys.argv[1]}.nam", "wb").write(bytes(nt_data)+b"\0"*0x40) # Empty Attributes for reference
    open(f"{sys.argv[1]}_pal.inc", "w").write(s_pal_gen[0])
    open(f"{sys.argv[1]}_pal.h", "w").write(s_pal_gen[1])
    open(f"{sys.argv[1]}_plt.pal", "wb").write(s_plt)
    open(f"{sys.argv[1]}_irq.inc", "w").write(f"_{sys.argv[6]}_irq:\n    .export _{sys.argv[6]}_irq\n    .byte ${',$'.join( pad(hex(o)[2:]).upper() for o in irq_data )}")    
    open(f"{sys.argv[1]}_irq.h", "w").write(f"extern const unsigned char {sys.argv[6]}_irq[];\n")    
    #open("test.bin", "wb").write(b"".join(ch_b))

