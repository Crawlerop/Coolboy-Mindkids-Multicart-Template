import typing
import struct


def compress(data: typing.Union[bytes, bytearray], bit_length: int = 1, rle_max: int = 127):
    assert (len(data) % bit_length) == 0

    last_bit = -1
    temp_length = []
    temp_data = []
    bit_offset = 0

    while bit_offset < len(data):
        if last_bit != data[bit_offset:bit_offset + bit_length]:
            temp_data.append(data[bit_offset:bit_offset + bit_length])
            temp_length.append(1)
            last_bit = data[bit_offset:bit_offset + bit_length]
        else:
            temp_length[-1] += 1
            if temp_length[-1] >= rle_max:
                last_bit = -1
        bit_offset += bit_length

    return temp_data, temp_length


def compress_pb(data: typing.Union[bytes, bytearray], bit_length: int = 1):
    cmp_data, cmp_len = compress(data, bit_length)
    assert len(cmp_data) == len(cmp_len)
    cmp_offset = 0
    cmp_temp_data = bytearray()

    output = bytearray()

    while cmp_offset < len(cmp_data):
        if cmp_len[cmp_offset] > 1:
            if len(cmp_temp_data) >= 1:
                output += struct.pack("<B", len(cmp_temp_data) + 0x80) + cmp_temp_data
            cmp_temp_data.clear()
            output += struct.pack("<B", cmp_len[cmp_offset]) + cmp_data[cmp_offset]
        else:
            cmp_temp_data += cmp_data[cmp_offset]
            if len(cmp_temp_data) >= 127:
                output += struct.pack("<B", len(cmp_temp_data) + 0x80) + cmp_temp_data
                cmp_temp_data.clear()
        cmp_offset += 1

    if len(cmp_temp_data) > 1:
        output += struct.pack("<B", len(cmp_temp_data) + 0x80) + cmp_temp_data

    return bytes(output)


if __name__ == "__main__":
    print(compress_pb(b"wrapperr"))
    print(compress(b"a"*256))
