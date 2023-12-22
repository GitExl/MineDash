from __future__ import annotations
from typing import Tuple, List, Sequence, Optional

from PIL.Image import Image

from bitpack import bitpack_4bpp


Color = Tuple[int, int, int]


class Palette:

    def __init__(self, name: str, colors: List[Color], data: bytearray):
        self._name: str = name
        self._data: bytearray = data
        self._colors: List[Color] = colors

    @staticmethod
    def from_image(name: str, img: Image) -> Palette:
        if img.width != 16 or img.height != 16:
            raise Exception('Palette is not 16x16.')

        if img.mode != 'RGB':
            img = img.convert('RGB')

        colors: List[Color] = []
        colors_vera: List[int] = []

        for color in img.getdata():
            r4 = (color[0] * 15 + 135) >> 8
            g4 = (color[1] * 15 + 135) >> 8
            b4 = (color[2] * 15 + 135) >> 8

            if color[0] != r4 * 17 or color[1] != g4 * 17 or color[2] != b4 * 17:
                print('Cannot represent color {} from {} in 4 bits per component.'.format(color, name))

            colors_vera.extend([g4, b4, 0, r4])
            colors.append(color)

        return Palette(name, colors, bitpack_4bpp(colors_vera))

    def find_subpalette_for_colors(self, colors: Sequence[Color]) -> Optional[int]:
        if len(self._colors) != 256:
            raise Exception('Cannot look for subpalettes in palettes smaller than 256 colors.')

        for subpal_index in range(0, 16):
            match_subpal = True

            for color in colors:
                match_color = False
                for color_index in range(subpal_index * 16, subpal_index * 16 + 16):
                    if self._colors[color_index] == color:
                        match_color = True
                        break

                if not match_color:
                    match_subpal = False
                    break

            if match_subpal:
                return subpal_index

        return None

    def colors_to_subindex(self, source: Sequence[Color], subpal_index: int) -> bytearray:
        tile_bin: bytearray = bytearray(len(source))

        start = subpal_index * 16
        end = subpal_index * 16 + 16
        for col_index, col in enumerate(source):
            for pal_index in range(start, end):
                if self._colors[pal_index] == col:
                    tile_bin[col_index] = pal_index - start
                    break

        return tile_bin

    def colors_to_index(self, source: Sequence[Color]) -> bytearray:
        tile_bin: bytearray = bytearray(len(source))

        for col_index, col in enumerate(source):
            for pal_index in range(0, 255):
                if self._colors[pal_index] == col:
                    tile_bin[col_index] = pal_index

        return tile_bin

    @property
    def name(self) -> str:
        return self._name

    @property
    def data(self) -> bytearray:
        return self._data
