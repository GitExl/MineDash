from __future__ import annotations
import json
import sys
from math import ceil, floor
from pathlib import Path
from typing import Dict, Optional, List

from PIL import Image

from binimage import BinImage
from palette import Palette
from sfx import SfxList
from spritegroup import SpriteGroup
from statelist import StateList
from tilemap import Tilemap
from tileset import TileSet


def load_palettes(config) -> Dict[str, Palette]:
    palettes: Dict[str, Palette] = {}

    for pal_name, pal_conf in config['palettes'].items():
        filename = 'res' / Path(pal_conf['file'])
        print('Reading palette {} from {}'.format(pal_name, filename))

        palettes[pal_name] = Palette.from_image(pal_name, Image.open(filename))

    return palettes


def write_palettes(palettes: Dict[str, Palette], destination: Path):
    for pal_name, pal in palettes.items():
        pal_path = destination / Path('{}.PAL'.format(pal_name.upper()))
        print('Writing palette {} to {}'.format(pal_name, pal_path))
        with open(pal_path, 'wb') as f:
            f.write(pal.data)


def load_tilesets(config, palettes: Dict[str, Palette]) -> Dict[str, TileSet]:
    tilesets: Dict[str, TileSet] = {}

    for tileset_name, conf in config['tilesets'].items():
        filename = 'res' / Path(conf['file'])
        print('Reading tileset {} from {}'.format(tileset_name, filename))

        palette: Optional[Palette] = None
        if 'palette' in conf:
            palette = palettes[conf['palette']]
            if palette is None:
                raise Exception('Unknown palette {} for tileset {}'.format(conf['palette'], tileset_name))

        tiles = Image.open(filename)
        tileset = TileSet.from_image(
            tileset_name,
            int(conf['address'], 0),
            tiles,
            conf['size'],
            conf['bpp'],
            palette
        )

        if 'metadata' in conf:
            with open('res' / Path(conf['metadata']), 'r') as f:
                metadata = json.load(f)
            tileset.set_metadata(metadata)

        tilesets[tileset_name] = tileset

    return tilesets


def write_tilesets(tilesets: Dict[str, TileSet], destination: Path, src: Path):

    for tileset_name, tileset in tilesets.items():

        # Write graphics.
        tileset_path = destination / Path('{}.TIL'.format(tileset_name.upper()))
        print('Writing tileset {} graphics to {}'.format(tileset_name, tileset_path))
        with open(tileset_path, 'wb') as f:
            tileset.write_data(f)

        if tileset.bpp == 1:
            continue

        # Write metadata.
        metadata_path = destination / Path('{}.TILDAT'.format(tileset_name.upper()))
        print('Writing tileset {} metadata to {}'.format(tileset_name, metadata_path))
        with open(metadata_path, 'wb') as f:
            tileset.write_metadata(f)

    # Write tile names.
    names_path = src / Path('tile_names.h')
    print('Writing tile names to {}'.format(names_path))
    with open(names_path, 'w') as f:
        f.write('// Auto-generated during build process.\n\n')
        f.write('#ifndef TILE_NAMES_H\n')
        f.write('#define TILE_NAMES_H\n')

        for tileset_name, tileset in tilesets.items():
            for name, index in tileset.tile_names.items():
                f.write('#define T_{}_{} {}\n'.format(tileset_name.upper(), name.upper(), index))
            f.write('\n')

        f.write('#endif\n')


def load_sprites(config, palettes: Dict[str, Palette]) -> Dict[str, SpriteGroup]:
    sprite_groups: Dict[str, SpriteGroup] = {}

    for group_name, group_conf in config['sprite_groups'].items():
        sprite_groups[group_name] = SpriteGroup(
            group_name,
            int(group_conf['address'], 0)
        )

    for sprite_name, sprite_conf in config['sprites'].items():
        filename = 'res' / Path(sprite_conf['file'])
        print('Reading sprite {} from {}'.format(sprite_name, filename))

        palette: Optional[Palette] = None
        if 'palette' in sprite_conf:
            palette = palettes[sprite_conf['palette']]
            if palette is None:
                raise Exception('Unknown palette {} for sprite {}'.format(sprite_conf['palette'], sprite_name))

        sprite_img = Image.open(filename)
        if sprite_img.mode != 'RGB':
            sprite_img = sprite_img.convert('RGB')

        x = sprite_conf['x']
        y = sprite_conf['y']
        width = sprite_conf['width']
        height = sprite_conf['height']

        count = 0
        sprites: Dict[str, BinImage] = {}
        while count < sprite_conf['count']:
            sprite_name_num = '{}_{:02}'.format(sprite_name, count)

            sprite_sub_img = sprite_img.crop((x, y, x + width, y + height))
            sprite_colors = sprite_sub_img.getdata()
            bin_img = BinImage.from_colors(width, height, sprite_conf['bpp'], sprite_colors, palette)

            if 'subpalette' in sprite_conf:
                bin_img.sub_palette = sprite_conf['subpalette']

            sprites[sprite_name_num] = bin_img

            count += 1
            x += width
            if x >= sprite_img.width:
                x = 0
                y += height

        for group_name in sprite_conf['groups']:
            group: SpriteGroup
            if group_name in sprite_groups:
                group = sprite_groups[group_name]
            else:
                raise Exception('Undefined sprite group {}.'.format(group_name))

            group.add_sprites(sprites)

    return sprite_groups


def write_sprites(sprite_groups: Dict[str, SpriteGroup], destination: Path, src: Path):

    for group_name, group in sprite_groups.items():

        # Write sprite graphics.
        group_path = destination / Path('{}.SPR'.format(group_name.upper()))
        print('Writing spritegroup {} graphics to {}'.format(group_name, group_path))
        with open(group_path, 'wb') as f:
            group.write_data(f)

        # Write sprite metadata.
        metadata_path = destination / Path('{}.SPRDAT'.format(group_name.upper()))
        print('Writing spritegroup {} metadata to {}'.format(group_name, metadata_path))
        with open(metadata_path, 'wb') as f:
            group.write_metadata(f)

    # Write sprite C header data.
    header_path = src / Path('sprite_types.h')
    print('Writing sprite types to {}'.format(header_path))
    with open(header_path, 'w') as f:
        f.write('// Auto-generated during build process.\n\n')
        f.write('#ifndef SPRITE_TYPES_H\n')
        f.write('#define SPRITE_TYPES_H\n\n')

        for group_name, group in sprite_groups.items():
            group.write_header_indices(f)

        f.write('#endif\n')


def load_tilemaps(config, tilesets: Dict[str, TileSet]) -> Dict[str, Tilemap]:
    tilemaps: Dict[str, Tilemap] = {}
    for tilemap_name, conf in config['tilemaps'].items():
        filename = 'res' / Path(conf['file'])
        print('Reading tilemap {} from {}'.format(tilemap_name, filename))

        tileset = tilesets[conf['tileset']]
        if tileset is None:
            raise Exception('Unknown tileset {} for tilemap {}'.format(conf['tileset'], tilemap_name))

        tilemaps[tilemap_name] = Tilemap.from_tmj(
            tilemap_name,
            conf['title'],
            tileset,
            filename
        )

    return tilemaps


def write_tilemaps(tilemaps: Dict[str, Tilemap], destination: Path, src: Path, entities: Dict[str, int]):

    # Write tile data.
    for tilemap_name, tilemap in tilemaps.items():
        tilemap_path = destination / Path('{}.MAP'.format(tilemap_name.upper()))
        print('Writing tilemap {} to {}'.format(tilemap_name, tilemap_path))
        with open(tilemap_path, 'wb') as f:
            tilemap.write_data(f)

        metadata_path = destination / Path('{}.DAT'.format(tilemap_name.upper()))
        print('Writing tilemap {} metadata to {}'.format(tilemap_name, metadata_path))
        with open(metadata_path, 'wb') as f:
            tilemap.write_metadata(f)

        entity_path = destination / Path('{}.ENT'.format(tilemap_name.upper()))
        print('Writing tilemap {} entities to {}'.format(tilemap_name, entity_path))
        with open(entity_path, 'wb') as f:
            tilemap.write_entities(f, entities)

    # Write level titles.
    titles_path = src / Path('level_names.c')
    print('Writing level names to {}'.format(titles_path))
    with open(titles_path, 'w') as f:
        f.write('// Auto-generated during build process.\n\n')
        f.write('char* level_names[] = {\n')
        for tilemap in tilemaps.values():
            title = tilemap.title.replace('"', '\\"').lower()
            f.write('  "{}",\n'.format(title))
        f.write('};\n')


def load_entities(config):
    entities: Dict[str, int] = {}
    for entity_name, entity_type in config['entities'].items():
        entities[entity_name] = entity_type

    return entities


def write_entities(entities: Dict[str, int], src: Path):
    entities_path = src / Path('entity_types.h')
    print('Writing entity types to {}'.format(entities_path))
    with open(entities_path, 'w') as f:
        f.write('// Auto-generated during build process.\n\n')
        f.write('#ifndef ENTITY_TYPES_H\n')
        f.write('#define ENTITY_TYPES_H\n\n')

        for entity_name, entity_type in entities.items():
            f.write('#define E_{} {}\n'.format(entity_name.upper(), entity_type))

        f.write('\n#endif\n')


def load_state_lists(config, sprite_groups: Dict[str, SpriteGroup]) -> Dict[str, StateList]:
    states: Dict[str, StateList] = {}

    for states_name, states_conf in config['states'].items():
        sprite_group = sprite_groups[states_conf['sprite_group']]

        filename = 'res' / Path(states_conf['file'])
        print('Reading statelist {} from {}'.format(states_name, filename))

        with open(filename, 'r') as f:
            states_data = json.load(f)
        states[states_name] = StateList.from_json(states_name, sprite_group, states_data)

    return states


def write_state_lists(state_lists: Dict[str, StateList], src: Path, destination: Path):
    for states_name, state_list in state_lists.items():

        # Struct data.
        state_list_path = destination / Path('{}.STA'.format(states_name.upper()))
        print('Writing statelist {} to {}'.format(states_name,state_list_path))
        with open(state_list_path, 'wb') as f:
            state_list.write_data(f)

    # State labels.
    labels_path = src / Path('state_labels.h')
    print('Writing state labels to {}'.format(labels_path))
    with open(labels_path, 'w') as f:
        f.write('// Auto-generated during build process.\n\n')
        f.write('#ifndef STATE_LABELS_H\n')
        f.write('#define STATE_LABELS_H\n\n')

        for state_list in state_lists.values():
            state_list.write_labels(f)
        f.write('\n')

        f.write('#endif\n')


def load_sfx_lists(config) -> Dict[str, SfxList]:
    sfxs: Dict[str, SfxList] = {}

    for sfx_name, sfx_conf in config['sfx'].items():
        filename = 'res' / Path(sfx_conf['file'])
        print('Reading SFX list {} from {}'.format(sfx_name, filename))

        with open(filename, 'r') as f:
            sfx_data = json.load(f)
        sfxs[sfx_name] = SfxList(sfx_name, sfx_data)

    return sfxs


def write_sfx_lists(sfx_lists: Dict[str, SfxList], src: Path, destination: Path):
    for sfx_name, sfx_list in sfx_lists.items():

        # Struct data.
        state_list_path = destination / Path('{}.SFX'.format(sfx_name.upper()))
        print('Writing SFX list {} to {}'.format(sfx_name,state_list_path))
        with open(state_list_path, 'wb') as f:
            sfx_list.write_data(f)

    # State labels.
    labels_path = src / Path('sfx_labels.h')
    print('Writing SFX labels to {}'.format(labels_path))
    with open(labels_path, 'w') as f:
        f.write('// Auto-generated during build process.\n\n')
        f.write('#ifndef SFX_LABELS_H\n')
        f.write('#define SFX_LABELS_H\n\n')

        for sfx_list in sfx_lists.values():
            sfx_list.write_labels(f)
        f.write('\n')

        f.write('#endif\n')


def write_pan_curves(src: Path):
    pan_l: List[float] = []
    pan_r: List[float] = []

    distx = -1.0
    while distx <= 1.0:
        if abs(distx) < 0.001:
            ax = 0.0
        else:
            ax = distx

        vl = 1.0 - abs(ax)
        vr = 1.0 - abs(ax)
        if ax > 0:
            vr = vr * vr
        elif ax < 0:
            vl = vl * vl

        pan_l.append(floor(vl * 63))
        pan_r.append(floor(vr * 63))

        distx += 1.0 / 63

    pan_path = src / Path('sfx_pan.c')
    print('Writing pan curves to {}'.format(pan_path))
    with open(pan_path, 'w') as f:
        f.write('// Auto-generated during build process.\n\n')

        f.write('unsigned char pan_l[128] = {\n')
        for v in pan_l:
            f.write('  {},\n'.format(v))
        f.write('};\n')

        f.write('\n')
        f.write('unsigned char pan_r[128] = {\n')
        for v in pan_r:
            f.write('  {},\n'.format(v))
        f.write('};\n')


def run():
    with open('res.json', 'r') as f:
        conf = json.load(f)

    destination = Path(conf['destination'])
    src = Path(conf['src'])

    palettes: Dict[str, Palette] = load_palettes(conf)
    tilesets: Dict[str, TileSet] = load_tilesets(conf, palettes)
    sprite_groups: Dict[str, SpriteGroup] = load_sprites(conf, palettes)
    tilemaps: Dict[str, Tilemap] = load_tilemaps(conf, tilesets)
    entities: Dict[str, int] = load_entities(conf)
    state_lists: Dict[str, StateList] = load_state_lists(conf, sprite_groups)
    sfx_lists: Dict[str, SfxList] = load_sfx_lists(conf)

    write_palettes(palettes, destination)
    write_tilesets(tilesets, destination, src)
    write_sprites(sprite_groups, destination, src)
    write_tilemaps(tilemaps, destination, src, entities)
    write_entities(entities, src)
    write_state_lists(state_lists, src, destination)
    write_sfx_lists(sfx_lists, src, destination)
    write_pan_curves(src)


if __name__ == '__main__':
    run()
