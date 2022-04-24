import struct
import audioop
import sys

indexTable = [
    -1, -1, -1, -1, 1, 2, 4, 6,
    -1, -1, -1, -1, 1, 2, 4, 6,
]

stepsizeTable = [
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767,
]

class State:
    def __init__(self):
        self.valprev = 0
        self.index = 0
    def encode(self,vl):
        # Encode Section #
        #print(vl)
        val = vl >> 4        

        step = stepsizeTable[self.index]
        ''' Step 1 - compute difference with previous value '''

        diff = val - self.valprev
        sign = 0
        if (diff < 0):
            sign = 8
        if (sign):
            diff = -diff

        ''' Step 2 - Divide and clamp '''

        '''
        Note:
        ** This code *approximately* computes:
        **    delta = diff*4/step;
        **    vpdiff = (delta+0.5)*step/4;
        ** but in shift step bits are dropped. The net result of this is
        ** that even if you have fast mul/div hardware you cannot put it to
        ** good use since the fixup would be too expensive.
        '''

        delta = 0
        vpdiff = (step >> 3)
        if ( diff >= step ):
            delta = 4
            diff -= step
            vpdiff += step
        step >>= 1
        if ( diff >= step ):
            delta |= 2
            diff -= step
            vpdiff += step
        step >>= 1
        if ( diff >= step ):
            delta |= 1
            vpdiff += step

        ''' Step 3 - Update previous value '''

        if (sign):
            self.valprev -= vpdiff
        else:
            self.valprev += vpdiff

        ''' Step 4 - Clamp previous value to 16 bits '''
        
        if ( self.valprev > 2047 ):
            self.valprev = 2047
        elif ( self.valprev < -2048 ):
            self.valprev = -2048

        ''' Step 5 - Assemble value, update index and step values '''
        
        delta |= sign

        # End Encode Section #

        self.index += indexTable[delta]
        if ( self.index < 0 ):
            self.index = 0
        if ( self.index > 88 ):
            self.index = 88

        step = stepsizeTable[self.index]

        # Write output #
        
        return delta

def u8_s8(a):    
    if (a <= 0x7f):
        #print(a)
        return a
    else:
        #print(a - 256)
        return a - 256

if __name__ == "__main__":
    s_dt = [u8_s8(a) for a in audioop.bias(open(sys.argv[1], "rb").read(), 1, 128)]    
    #s_dt = open(sys.argv[1], "rb").read()
    #open("s_tmp", "wb").write(s_dt)

    adp_state = State()
    out_nib = [adp_state.encode(x << 8) for x in s_dt]
    
    out_mode = 0
    out_tmp = 0

    out_buf = bytearray()

    for a in out_nib:
        if out_mode == 1:
            out_buf.append((out_tmp << 4) | a)
            out_tmp = 0
        else:
            out_tmp = a
        out_mode = (out_mode + 1) & 1

    if out_mode == 1:
        out_buf.append(out_tmp << 4)

    open(sys.argv[2], "wb").write(out_buf)