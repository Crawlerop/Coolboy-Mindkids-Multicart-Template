import rle_nes
import sys

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} [file] [name]")
        sys.exit(1)

    s_data = rle_nes.compress_pb(open(sys.argv[1], "rb").read())
    print(f"// usage: extern const char {sys.argv[2]}[{len(s_data)}];")
    print(f"//        extern const int {sys.argv[2]}_size = {len(s_data)};");
    print()
    print("// DATA BANK")
    print(f"const char {sys.argv[2]}[{len(s_data)}] = "+"{")
    for s in s_data:
         print(f"{hex(s)}, ", end="")
    print("\n};\n\n// FIXED DATA\n"+f"const int {sys.argv[2]}_size = {len(s_data)};")