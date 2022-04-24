import wave
import audioop
import sys
import struct

if __name__ == "__main__":
    wav: wave.Wave_read = wave.open(sys.argv[1])
    assert wav.getnchannels() == 1
    assert wav.getframerate() == 16000
    
    data = wav.readframes(wav.getnframes())
    
    data_downsampled = []    
    if wav.getsampwidth() == 2:
        d_offs = 0
        while d_offs<len(data):
            data_downsampled.append(struct.unpack("<h", data[d_offs:d_offs+2])[0] >> 8)
            d_offs += 2
    else:
        data_downsampled = [struct.unpack("<b", bytes([x]))[0] for x in audioop.bias(data, 1, 128)]

    data_clamped = [(126 if x > 126 else (-127 if x < -127 else x)) for x in data_downsampled]
    data_packed = b"".join([struct.pack("<b", x) for x in data_clamped])
    #print(data_clamped[:16])
    open(sys.argv[2], "wb").write(audioop.bias(bytes([x for x in data_packed]), 1, 128))
