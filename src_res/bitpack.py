from typing import Sequence


def bitpack_1bpp(data: Sequence[int]):
    if len(data) % 8:
        raise Exception('Cannot bitpack data to 1bpp that is not a multiple of 8 bytes.')

    packed: bytearray = bytearray(len(data) // 8)

    byte_index = 0
    bit = 0x80
    for col in data:
        if col:
            packed[byte_index] |= bit
        if bit == 0x01:
            bit = 0x80
            byte_index += 1
        else:
            bit >>= 1

    return packed


def bitpack_2bpp(data: Sequence[int]):
    if len(data) % 8:
        raise Exception('Cannot bitpack data to 2bpp that is not a multiple of 4 bytes.')

    packed: bytearray = bytearray(len(data) // 4)

    byte_index = 0
    shift = 0
    for col in data:
        packed[byte_index] |= col << shift
        if shift == 6:
            shift = 0
            byte_index += 1
        else:
            shift += 2

    return packed


def bitpack_4bpp(data: Sequence[int]):
    if len(data) % 8:
        raise Exception('Cannot bitpack data to 4bpp that is not a multiple of 2 bytes.')

    packed: bytearray = bytearray(len(data) // 2)

    byte_index = 0
    shift = 4
    for col in data:
        packed[byte_index] |= col << shift
        if shift == 0:
            shift = 4
            byte_index += 1
        else:
            shift = 0

    return packed
