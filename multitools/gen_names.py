import sys

LENGTH = 16
out_file = open(sys.argv[2], "wb")
for l in open(sys.argv[1]):
    name = l.rstrip().encode("ascii").upper()[:LENGTH]
    out_file.write(name + b"\0"*(LENGTH-len(name)))