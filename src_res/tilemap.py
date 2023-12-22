from __future__ import annotations
import json
import struct
from pathlib import Path
from typing import Tuple, Optional, BinaryIO, List, Dict

from tileset import TileSet


TilemapBounds = Tuple[int, int, int, int]


ENTITIES_MAX = 64


class EntityFlags:

    UNUSED = 0x01
    NO_OWNER = 0x02
    INVISIBLE = 0x04


class Entity:

    def __init__(self, kind: str, flags: int, x: int, y: int):
        self.kind: str = kind
        self.flags: int = flags
        self.x: int = x
        self.y: int = y


class Tilemap:

    def __init__(self, name: str, title: str, tileset: TileSet, tiles: bytearray, width: int, height: int, points: int, time: int, entities: List[Entity], bounds: Optional[TilemapBounds] = None):
        self._name: str = name
        self._title: str = title
        self._tileset: TileSet = tileset
        self._tiles: bytearray = tiles
        self._width: int = width
        self._height: int = height
        self._points: int = points
        self._time: int = time
        self._bounds: Optional[TilemapBounds] = bounds
        self._entities: List[Entity] = entities

    @staticmethod
    def from_tmj(name: str, title: str, tileset: TileSet, path: Path) -> Tilemap:
        with open(path, 'r') as f:
            data = json.load(f)

        width: int = data['width']
        height: int = data['height']
        if width != 64:
            raise Exception('Tilemap {} has an invalid width of {}.'.format(name, width))
        if height != 64:
            raise Exception('Tilemap {} has an invalid height of {}.'.format(name, height))

        tiles: Optional[bytearray] = None
        entities: List[Entity] = []
        bounds: Optional[TilemapBounds] = None
        points: int = 50
        time: int = 60

        for prop in data['properties']:
            if prop['name'] == 'Points':
                points = prop['value']
            elif prop['name'] == 'Time':
                time = prop['value']
            else:
                raise Exception('Tilemap {} has unknown property {}.'.format(name, prop['name']))

        for layer in data['layers']:

            # Generate VERA tile data from first tile layer.
            if tiles is None and layer['type'] == 'tilelayer':
                tiles_raw = layer['data']
                tiles = bytearray(len(tiles_raw) * 2)
                for index, tile_raw in enumerate(tiles_raw):
                    flip_h = 0x04 if tile_raw & 0x80000000 else 0
                    flip_v = 0x08 if tile_raw & 0x40000000 else 0
                    tile = tile_raw & 0xFFFF
                    if tile > 0:
                        tile -= 1

                    if tile < 0 or tile > 1023:
                        raise Exception('Invalid tile index {} in {}.'.format(tile, name))
                    if tile >= len(tileset.tiles):
                        raise Exception('Tile index {} in {} not in tileset {}.'.format(tile, name, tileset.name))

                    sub_palette = tileset.tiles[tile].sub_palette << 4
                    tiles[index * 2 + 0] = tile & 0xFF
                    tiles[index * 2 + 1] = (tile >> 8) & 0x03 | flip_h | flip_v | sub_palette

            # Extract metadata from object layers.
            elif layer['type'] == 'objectgroup':
                for obj in layer['objects']:
                    if obj['type'] == 'Bounds':
                        bounds = (obj['x'], obj['y'], obj['x'] + obj['width'], obj['y'] + obj['height'])
                    else:
                        entities.append(Entity(obj['type'], 0, obj['x'], obj['y']))

            else:
                raise Exception('Unknown layer type {} in tilemap {}.'.format(layer['type'], name))

        if tiles is None:
            raise Exception('Tilemap {} does not contain any tiles.'.format(name))

        return Tilemap(name, title, tileset, tiles, width, height, points, time, entities, bounds)

    def write_data(self, f: BinaryIO):
        f.write(self._tiles)

    def write_metadata(self, f: BinaryIO):
        minutes = self._time // 60
        seconds = self._time % 60

        s = struct.Struct('HHHHhBB')
        f.write(s.pack(
            self._bounds[0],
            self._bounds[1],
            self._bounds[2],
            self._bounds[3],
            self._points,
            minutes,
            seconds,
        ))

    def write_entities(self, f: BinaryIO, entity_types: Dict[str, int]):
        pad_count = ENTITIES_MAX - len(self._entities)

        # Type
        s = struct.Struct('B')
        for entity in self._entities:
            if entity.kind not in entity_types:
                raise Exception('Unknown entity type {}.'.format(entity.kind))
            f.write(s.pack(entity_types[entity.kind]))
        for pad in range(0, pad_count):
            f.write(b'\x00')

        # Flags
        s = struct.Struct('B')
        for entity in self._entities:
            f.write(s.pack(entity.flags))
        for pad in range(0, pad_count):
            f.write(s.pack(EntityFlags.UNUSED))

        # Data
        s = struct.Struct('B')
        for entity in self._entities:
            f.write(s.pack(0))
        for pad in range(0, pad_count):
            f.write(b'\x00')

        # State
        s = struct.Struct('B')
        for entity in self._entities:
            f.write(s.pack(0))
        for pad in range(0, pad_count):
            f.write(b'\x00')

        # Counter
        s = struct.Struct('B')
        for entity in self._entities:
            f.write(s.pack(0))
        for pad in range(0, pad_count):
            f.write(b'\x00')

        # X
        s = struct.Struct('b')
        for entity in self._entities:
            f.write(s.pack(entity.x % 16))
        for pad in range(0, pad_count):
            f.write(b'\x00')

        # Y
        s = struct.Struct('b')
        for entity in self._entities:
            f.write(s.pack(entity.y % 16))
        for pad in range(0, pad_count):
            f.write(b'\x00')

        # Tile X
        s = struct.Struct('B')
        for entity in self._entities:
            f.write(s.pack(entity.x // 16))
        for pad in range(0, pad_count):
            f.write(b'\x00')

        # Tile Y
        s = struct.Struct('B')
        for entity in self._entities:
            f.write(s.pack(entity.y // 16))
        for pad in range(0, pad_count):
            f.write(b'\x00')

    @property
    def title(self) -> str:
        return self._title
