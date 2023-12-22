import struct
from typing import Dict, TextIO, BinaryIO

from binimage import BinImage


SPRITES_MAX = 128


class SpriteGroup:

    def __init__(self, name: str, address: int):
        self._name: str = name
        self._address: int = address

        self._sprites: Dict[str, BinImage] = {}
        self._sprite_indices: Dict[str, int] = {}

    def index_for_name(self, name: str) -> int:
        return self._sprite_indices[name]

    def name_for_sprite(self, name: str) -> str:
        return 'SP_{}_{}'.format(self._name, name)

    def add_sprites(self, img: Dict[str, BinImage]):
        index = len(self._sprites)
        for name, img in img.items():
            self._sprites[name] = img
            self._sprite_indices[name] = index
            index += 1

        if len(self._sprites) > SPRITES_MAX:
            raise Exception('Too many sprites.')

    def write_data(self, f: BinaryIO):
        s = struct.Struct('H')
        f.write(s.pack(self._address & 0xFFFF))

        for sprite_name, sprite in self._sprites.items():
            f.write(sprite.data)

    def write_header_indices(self, f: TextIO):
        for sprite_name, sprite in self._sprites.items():
            f.write('#define {} {}\n'.format(self.name_for_sprite(sprite_name), self.index_for_name(sprite_name)))
        f.write('\n')

    def write_metadata(self, f: BinaryIO):

        # Write sprite data offsets.
        s = struct.Struct('BB')
        offset = self._address
        for sprite_name, sprite in self._sprites.items():
            addr0 = (offset >> 5) & 0xFF
            addr1 = (offset >> 13) & 0xFF
            if sprite.bpp == 8:
                addr1 |= 0x80

            f.write(s.pack(addr0, addr1))
            offset += len(sprite.data)

        s = struct.Struct('BB')
        for pad in range(0, SPRITES_MAX - len(self._sprites)):
            f.write(s.pack(0, 0))

        # Write size and palettes.
        s = struct.Struct('B')
        for sprite_name, sprite in self._sprites.items():
            val = sprite.sub_palette

            if sprite.height == 8:
                val |= 0b00000000
            elif sprite.height == 16:
                val |= 0b01000000
            elif sprite.height == 32:
                val |= 0b10000000
            elif sprite.height == 64:
                val |= 0b11000000
            else:
                raise Exception('Sprite {} has invalid height {}.'.format(sprite_name, sprite.height))

            if sprite.width == 8:
                val |= 0b00000000
            elif sprite.width == 16:
                val |= 0b00010000
            elif sprite.width == 32:
                val |= 0b00100000
            elif sprite.width == 64:
                val |= 0b00110000
            else:
                raise Exception('Sprite {} has invalid width {}.'.format(sprite_name, sprite.width))

            f.write(s.pack(val))

        for pad in range(0, SPRITES_MAX - len(self._sprites)):
            f.write(s.pack(0))

    @property
    def name(self) -> str:
        return self._name
