from __future__ import annotations

import struct
from typing import List, Optional, BinaryIO, Dict

from PIL.Image import Image

from binimage import BinImage
from palette import Palette


TILES_MAX = 64


class TileFlags:
    BLOCKS = 0x01
    SOFT = 0x02
    LETHAL = 0x04
    SPECIAL = 0x08
    DESTRUCTIBLE = 0x10
    GRAVITY = 0x20
    PUSHABLE = 0x40


class TileSet:

    def __init__(self, name: str, address: int, bpp: int):
        self._name: str = name
        self._address: int = address
        self._tiles: List[BinImage] = []
        self._tile_flags: List[int] = []
        self._tile_names: Dict[str, int] = {}
        self._bpp: int = bpp

    @staticmethod
    def from_image(name: str, address: int, img: Image, tile_size: int, bpp: int, palette: Optional[Palette] = None) -> TileSet:
        if img.width % tile_size:
            raise Exception('Tileset image width is not a multiple of {}.'.format(tile_size))
        if img.height % tile_size:
            raise Exception('Tileset image height is not a multiple of {}.'.format(tile_size))

        if img.mode != 'RGB':
            img = img.convert('RGB')

        tileset = TileSet(name, address, bpp)

        for y in range(0, img.height, tile_size):
            for x in range(0, img.width, tile_size):
                tile_img = img.crop((x, y, x + tile_size, y + tile_size))
                tile_colors = tile_img.getdata()
                tileset.tiles.append(BinImage.from_colors(img.width, img.height, bpp, tile_colors, palette))
                tileset.tile_flags.append(0)

        return tileset

    def set_metadata(self, data):
        if len(data['tiles']) > len(self._tiles):
            raise Exception('Tileset {} has more tile metadata than tiles.'.format(self._name))

        for index, tile in enumerate(data['tiles']):
            if 'type' in tile:
                self._tile_names[tile['type']] = index

            flags = 0
            for prop in tile['properties']:
                if not prop['value']:
                    continue

                if prop['name'] == 'Block':
                    flags |= TileFlags.BLOCKS
                elif prop['name'] == 'Special':
                    flags |= TileFlags.SPECIAL
                elif prop['name'] == 'Soft':
                    flags |= TileFlags.SOFT
                elif prop['name'] == 'Lethal':
                    flags |= TileFlags.LETHAL
                elif prop['name'] == 'Destructible':
                    flags |= TileFlags.DESTRUCTIBLE
                elif prop['name'] == 'Gravity':
                    flags |= TileFlags.GRAVITY
                elif prop['name'] == 'Pushable':
                    flags |= TileFlags.PUSHABLE
                else:
                    raise Exception('Unknown tile property {} in tileset {}.'.format(prop['name'], self._name))

            self._tile_flags[index] = flags

    def write_data(self, f: BinaryIO):
        s = struct.Struct('H')
        f.write(s.pack(self._address & 0xFFFF))

        for tile in self._tiles:
            f.write(tile.data)

    def write_metadata(self, f: BinaryIO):

        # Tile palettes.
        s = struct.Struct('B')
        for tile in self._tiles:
            f.write(s.pack(tile.sub_palette))
        for pad in range(0, TILES_MAX - len(self._tiles)):
            f.write(s.pack(0))

        # Tile flags.
        s = struct.Struct('B')
        for flags in self._tile_flags:
            f.write(s.pack(flags))
        for pad in range(0, TILES_MAX - len(self._tiles)):
            f.write(s.pack(0))

    @property
    def name(self) -> str:
        return self._name

    @property
    def tiles(self) -> List[BinImage]:
        return self._tiles

    @property
    def tile_flags(self) -> List[int]:
        return self._tile_flags

    @property
    def bpp(self) -> int:
        return self._bpp

    @property
    def tile_names(self) -> Dict[str, int]:
        return self._tile_names
