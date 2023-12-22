from __future__ import annotations
from typing import Optional, Sequence

from bitpack import bitpack_2bpp, bitpack_4bpp, bitpack_1bpp
from palette import Color, Palette


class BinImage:

    def __init__(self, width: int, height: int, bpp: int, packed_data: bytes, sub_palette: Optional[int] = None):
        self._data: bytes = packed_data
        self._bpp: int = bpp
        self._width: int = width
        self._height: int = height
        self.sub_palette: Optional[int] = sub_palette

    @staticmethod
    def from_colors(width: int, height: int, bpp: int, colors: Sequence[Color], palette: Optional[Palette] = None) -> BinImage:
        if bpp > 1 and palette is None:
            raise Exception('Cannot create BinImage from colors with bpp > 1 but without a palette.')

        # Use entire palette.
        if bpp == 8:
            tile_data_indexed = palette.colors_to_index(colors)
            return BinImage(bpp, width, height, tile_data_indexed)

        # Find first matching subpalette.
        elif bpp == 2 or bpp == 4:
            subpal_index = palette.find_subpalette_for_colors(colors)
            if subpal_index is None:
                raise Exception('Cannot find subpalette.')

            # Index colors and bitpack.
            tile_data_indexed = palette.colors_to_subindex(colors, subpal_index)
            if bpp == 2:
                tile_data_indexed = bitpack_2bpp(tile_data_indexed)
            elif bpp == 4:
                tile_data_indexed = bitpack_4bpp(tile_data_indexed)

            return BinImage(width, height, bpp, tile_data_indexed, subpal_index)

        # Convert to 1bpp bitpacked data directly.
        elif bpp == 1:
            tile_data_indexed: bytearray = bytearray(len(colors))
            for index, color in enumerate(colors):
                tile_data_indexed[index] = 1 if color[0] != 0 else 0
            tile_data_indexed = bitpack_1bpp(tile_data_indexed)

            return BinImage(width, height, bpp, tile_data_indexed)

        raise Exception('Cannot create BinImage from colors with bpp {}'.format(bpp))

    @property
    def data(self) -> bytes:
        return self._data

    @property
    def bpp(self) -> int:
        return self._bpp

    @property
    def width(self) -> int:
        return self._width

    @property
    def height(self) -> int:
        return self._height
