import rle_nes
import sys

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} [file] [output]")
        sys.exit(1)

    s_data = rle_nes.compress_pb(open(sys.argv[1], "rb").read())
    open(sys.argv[2], "wb").write(s_data+b"\x80\0")
    #print(f"Compressed Size: {len(s_data)}")