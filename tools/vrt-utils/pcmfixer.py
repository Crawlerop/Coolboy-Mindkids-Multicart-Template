import audioop
import sys
import struct

if __name__ == "__main__":
    f = open(sys.argv[1], "rb").read()    
    #f = b"\x02"    
    data_swapped = [struct.unpack("<b", bytes([x]))[0] for x in audioop.bias(f, 1, 128)]    
    
    data_clamped = [(126 if x > 126 else (-127 if x < -127 else x)) for x in data_swapped]
    
    data_packed = b"".join([struct.pack("<b", x) for x in data_clamped])    

    #print(data_clamped[:16])
    #print(data_packed)
    open(sys.argv[2], "wb").write(audioop.bias(bytes([x for x in data_packed]), 1, 128))
