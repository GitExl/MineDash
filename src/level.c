#include <cx16.h>
#include <cbm.h>
#include <stdlib.h>
#include <string.h>

#include "camera.h"
#include "text.h"
#include "level.h"
#include "sprites.h"
#include "entities.h"
#include "entity_types.h"
#include "state_labels.h"
#include "tile_names.h"
#include "level_names.h"
#include "main.h"
#include "sfx.h"
#include "sfx_labels.h"

#include "exit.h"
#include "player.h"
#include "faller.h"

// Frames per second of level clock. Slightly faster than real seconds for a sense of urgency.
#define LEVEL_SECOND 50

// Current level index.
unsigned char level_current = 0;

// Next level to load.
unsigned char level_next = 0;

// Level metadata.
level_info_t level_info;

// Level map tiles.
map_t map;

// Tileset metadata.
tileset_t tileset;

// 60Hz timer for updating the level clock.
unsigned char level_clock;

// Set if the HUD needs to be updated this frame.
unsigned char level_hud_update;

// Module shared variables.
static unsigned int tile_index;
static unsigned char tile_flags;
static unsigned char tile;
static unsigned char title_len;
static unsigned char flags;
static unsigned char pattern;

void level_load(const unsigned char level) {
  register unsigned char i, j;
  static unsigned char type_flags;

  char filename[] = "lvl000.map";

  level_current = level;
  level_clock = LEVEL_SECOND;
  level_hud_update = 1;

  // Base filename for map index.
  if (level > 99) {
    i = 3;
  } else if (level > 9) {
    i = 4;
  } else {
    i = 5;
  }
  utoa(level, &filename[0] + i, 10);
  filename[6] = '.';

  // Load level VERA tilemap, max 64x64x2 = 8k.
  cbm_k_setnam(&filename[0]);
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(2, 0x8000);

  // Load level metadata.
  filename[7] = 'd';
  filename[8] = 'a';
  filename[9] = 't';
  cbm_k_setnam(&filename[0]);
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(0, (unsigned int)&level_info);

  // Offset display area from visible bounds.
  level_info.right -= CAMERA_WIDTH;
  level_info.bottom -= CAMERA_HEIGHT;

  // Configure layer 0 for the loaded tilemap.
  VERA.layer0.config = 0b01010010;
  VERA.layer0.mapbase = 0x40;
  VERA.layer0.tilebase = 0x30 | 0b11;

  // Copy VERA level tiles to system RAM.
  // Also clear entity ownership.
  VERA.address = 0x8000;
  VERA.address_hi = VERA_INC_2;
  tile_index = 0;
  for (i = 0; i < 64; i++) {
    for (j = 0; j < 64; j++) {
      map.tile[tile_index] = VERA.data0;
      map.owner[++tile_index] = 0xFF;
    }
  }

  player.tnt = 0;
  exit_reset();

  filename[7] = 'e';
  filename[8] = 'n';
  filename[9] = 't';
  entities_load(filename);

  for (i = 0; i < ENTITY_MAX; i++) {
    flags = entities.flags[i];
    if (flags & ENTITYF_UNUSED) {
      continue;
    }

    // Set tile ownership.
    type_flags = entity_types.flags[entities.type[i]];
    if (type_flags & ETF_OWNERSHIP) {
      tile_index = TILE_INDEX(entities.tile_x[i], entities.tile_y[i]);
      map.owner[tile_index] = i;
    }

    // Track player entity index.
    if (entities.type[i] == E_PLAYER) {
      player.entity = i;
    }
  }
}

void level_load_graphics() {
  cbm_k_setnam("lvl.spr");
  cbm_k_setlfs(0, 8, 1);
  cbm_k_load(2, 0);

  cbm_k_setnam("lvl.sprdat");
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(0, (unsigned int)&sprites);

  cbm_k_setnam("lvl.til");
  cbm_k_setlfs(0, 8, 1);
  cbm_k_load(2, 0);

  cbm_k_setnam("lvl.tildat");
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(0, (unsigned int)&tileset);
}

const unsigned char DIRT_OFFSETS[] = {
  MAP_WIDTH - 1,
  2,
  MAP_WIDTH - 1,
  0,
};
const unsigned char DIRT_PATTERNS[] = {
  0b1000,
  0b0010,
  0b0001,
  0b0100,
};
const unsigned char DIRT_TILES[16] = {
  T_LVL_DIRT_MID,
  T_LVL_DIRT_CAPLEFT,
  T_LVL_DIRT_CAPRIGHT,
  T_LVL_DIRT_HORIZONTAL,
  T_LVL_DIRT_CAPTOP,
  T_LVL_DIRT_TOPLEFT,
  T_LVL_DIRT_TOPRIGHT,
  T_LVL_DIRT_TOP,
  T_LVL_DIRT_CAPBOTTOM,
  T_LVL_DIRT_BOTTOMLEFT,
  T_LVL_DIRT_BOTTOMRIGHT,
  T_LVL_DIRT_BOTTOM,
  T_LVL_DIRT_VERTICAL,
  T_LVL_DIRT_LEFT,
  T_LVL_DIRT_RIGHT,
  T_LVL_DIRT,
};
void level_tile_clear(const unsigned char tile_x, const unsigned char tile_y) {
  register unsigned char i, j;
  unsigned int tile_index_check;
  unsigned int tile_index;
  unsigned char map_tile;

  level_tile_set(TILE_INDEX(tile_x, tile_y), 0);

  // Check for gravity objects above and to the sides of the tile.
  level_tile_evaluate_faller(tile_x, tile_y - 1, GF_ABOVE);
  level_tile_evaluate_faller(tile_x - 1, tile_y, GF_LEFT);
  level_tile_evaluate_faller(tile_x - 1, tile_y - 1, GF_LEFT);
  level_tile_evaluate_faller(tile_x + 1, tile_y, GF_RIGHT);
  level_tile_evaluate_faller(tile_x + 1, tile_y - 1, GF_RIGHT);

  // Evaluate dirt tiles that should have soft borders.
  tile_index_check = TILE_INDEX(tile_x, tile_y - 1);
  for (i = 0; i < 4; i++) {

    // Only process soft tiles.
    map_tile = map.tile[tile_index_check];
    if (tileset.flags[map_tile] & TILEF_SOFT) {

      // Determines a pattern from 4 dirt tiles around this tile.
      pattern = 0;
      tile_index = tile_index_check - MAP_WIDTH;
      for (j = 0; j < 4; j++) {
        map_tile = map.tile[tile_index];
        if (tileset.flags[map_tile] & TILEF_SOFT) {
          pattern |= DIRT_PATTERNS[j];
        }
        tile_index += DIRT_OFFSETS[j];
      }
      level_tile_set(tile_index_check, DIRT_TILES[pattern]);
    }

    tile_index_check += DIRT_OFFSETS[i];
  }
}

unsigned char level_tile_push(const unsigned char tile_x, const unsigned char tile_y, const signed char direction) {
  static unsigned char type;

  tile_index = TILE_INDEX(tile_x, tile_y);
  tile = map.tile[tile_index];
  if (!(tileset.flags[tile] & TILEF_PUSHABLE)) {
    return 0;
  }

  // Check tile behind pushable.
  tile_index += direction;
  if (map.tile[tile_index] || map.owner[tile_index] != 0xFF) {
    return 0;
  }

  type = faller_type_for_tile(tile);
  entities_spawn(E_FALLER, tile_x, tile_y, 0, type | (direction < 0 ? FALLER_STATE_PUSH_LEFT : FALLER_STATE_PUSH_RIGHT));

  return 1;
}

unsigned char level_tile_flags(const unsigned char tile_x, const unsigned char tile_y) {
  tile_index = TILE_INDEX(tile_x, tile_y);

  tile = map.tile[tile_index];
  return tileset.flags[tile];
}

unsigned char level_tile_is_blocked(const unsigned char tile_x, const unsigned char tile_y) {
  tile_index = TILE_INDEX(tile_x, tile_y);

  tile = map.tile[tile_index];
  tile_flags = tileset.flags[tile];
  return (tile_flags & TILEF_BLOCKS) || (map.owner[tile_index] != 0xFF);
}

void level_tile_evaluate_faller(const unsigned char tile_x, const unsigned char tile_y, const unsigned char gravity_flags) {
  static unsigned char entity;
  static unsigned char type;
  static unsigned char tile;
  static unsigned char state;

  tile_index = TILE_INDEX(tile_x, tile_y);
  tile = map.tile[tile_index];
  tile_flags = tileset.flags[tile];
  if (!(tile_flags & TILEF_GRAVITY)) {
    return;
  }

  state = level_tile_evaluate_gravity(tile_index, gravity_flags);
  if (state) {
    type = faller_type_for_tile(tile);
    entity = entities_spawn(E_FALLER, tile_x, tile_y, 0, type | state | 2);
    if (state == FALLER_STATE_ROLL_LEFT) {
      entities.flags[entity] |= ENTITYF_FLIPX;
    }
  }
}

unsigned char level_tile_evaluate_gravity(unsigned int tile_index, const unsigned char gravity_flags) {

  // Slide to right.
  if (gravity_flags & GF_LEFT && level_tile_can_roll(tile_index, 1)) {
    return FALLER_STATE_ROLL_RIGHT;
  }

  // Slide to left.
  if (gravity_flags & GF_RIGHT && level_tile_can_roll(tile_index, -1)) {
    return FALLER_STATE_ROLL_LEFT;
  }

  // Fall down.
  if (gravity_flags & GF_ABOVE) {
    tile_index += MAP_WIDTH;
    if (!map.tile[tile_index]) {

      // Check for crushing.
      if (gravity_flags & GF_CRUSH && map.owner[tile_index] != 0xFF) {
        entities_crush(map.owner[tile_index]);
      }

      // Fall, if now possible.
      if (map.owner[tile_index] == 0xFF) {
        return FALLER_STATE_FALL;
      }
    }
  }

  return 0;
}

unsigned char level_tile_can_roll(unsigned int tile_index, const signed char side) {

  // The tile below must be a gravity affected tile.
  tile = map.tile[tile_index + MAP_WIDTH];
  if (tileset.flags[tile] & TILEF_GRAVITY) {

    // The side tile must be empty.
    tile_index += side;
    if (!map.tile[tile_index] && map.owner[tile_index] == 0xFF) {

      // The tile below and to the side must be empty.
      tile_index += MAP_WIDTH;
      if (!map.tile[tile_index] && map.owner[tile_index] == 0xFF) {
        return 1;
      }
    }
  }

  return 0;
}

void level_tile_execute_special(const unsigned char tile_x, const unsigned char tile_y) {
  tile_index = TILE_INDEX(tile_x, tile_y);
  tile = map.tile[tile_index];

  switch (tile) {

    // Gold.
    case T_LVL_GOLD:
      if (level_info.points) {
        level_info.points -= 50;
        if (level_info.points <= 0) {
          level_info.points = 0;
          exit_open();
        }
      }
      level_hud_update = 1;

      entities_set_state(entities_spawn(E_ANIM, tile_x, tile_y, 0, 0), ST_LVL_GOLD_TAKE);
      sfx_play_pan(SFX_LVL_GOLD_TAKE, 0x20, tile_x, tile_y);
      break;

    // Diamond.
    case T_LVL_DIAMOND:
      if (level_info.points) {
        level_info.points -= 100;
        if (level_info.points <= 0) {
          level_info.points = 0;
          exit_open();
        }
      }
      level_hud_update = 1;

      entities_set_state(entities_spawn(E_ANIM, tile_x, tile_y, 0, 0), ST_LVL_DIAMOND_TAKE);
      sfx_play_pan(SFX_LVL_GOLD_TAKE, 0x20, tile_x, tile_y);
      break;

    // TNT.
    case T_LVL_TNT:
      player.tnt++;
      level_hud_update = 1;

      entities_set_state(entities_spawn(E_ANIM, tile_x, tile_y, 0, 0), ST_LVL_TNT_TAKE);
      sfx_play_pan(SFX_LVL_TNT_TAKE, 0x20, tile_x, tile_y);
      break;

    // Info.
    case T_LVL_INFO:
      game_action = GAMEACTION_SHOW_INFO;
      sfx_play_pan(SFX_LVL_TNT_TAKE, 0x20, tile_x, tile_y);
      break;
  }

  level_tile_clear(tile_x, tile_y);
}

void level_tile_set(const unsigned int tile_index, const unsigned char tile) {
  map.tile[tile_index] = tile;

  VERA.address = 0x8000 + tile_index * 2;
  VERA.address_hi = VERA_INC_1;
  VERA.data0 = tile;
  VERA.data0 = tileset.palette[tile] << 4;
}

void level_hud_build() {
  title_len = strlen(level_names[level_current]);

  text_hud_clear();

  text_write(1, 27, 0b11110011, "\x001");
  text_write(1, 28, 0b11110011, "\x002");
  text_write(12, 27, 0b11110011, "\x003");
  text_write(39 - title_len, 27, 0b11110011, level_names[level_current]);
}

void level_update() {
  static unsigned char o;
  static char s[] = "     \x000";
  static char t[] = "00:00";
  static char tt[] = "0  ";

  // Advance clock.
  if (!(entities.data[entity_player] & PLAYER_DATA_DISABLED)) {
    --level_clock;
    if (!level_clock) {
      if (!level_info.time_seconds) {
        if (!level_info.time_minutes) {
          player_kill(entity_player, PLAYER_KILL_TIMEOUT);
        } else {
          level_info.time_seconds = 59;
          --level_info.time_minutes;
        }
      } else {
        --level_info.time_seconds;

        if (level_info.time_minutes == 0 && level_info.time_seconds < 15) {
          sfx_play(SFX_LVL_TIME_OUT, 63, 63, 0x10);
        }
      }

      level_clock = LEVEL_SECOND;
      level_hud_update = 1;
    }
  }

  if (level_hud_update) {

    // Points left.
    s[0] = ' ';
    s[1] = ' ';
    s[2] = ' ';
    s[3] = ' ';
    text_write(3, 27, 0b11110010, s);
    utoa(level_info.points, &s[0], 10);
    text_write(3, 27, 0b11110010, s);

    // Time left.
    t[0] = '0';
    t[1] = '0';
    t[3] = '0';
    t[4] = '0';
    o = level_info.time_minutes > 9 ? 0 : 1;
    utoa(level_info.time_minutes, &t[o], 10);
    o = level_info.time_seconds > 9 ? 3 : 4;
    utoa(level_info.time_seconds, &t[o], 10);
    t[2] = ':';
    text_write(3, 28, 0b11110010, t);

    // TNT.
    tt[0] = ' ';
    tt[1] = ' ';
    tt[2] = ' ';
    text_write(14, 27, 0b11110010, tt);
    utoa(player.tnt, &tt[0], 10);
    text_write(14, 27, 0b11110010, tt);

    level_hud_update = 0;
  }
}

void level_tile_touch(const unsigned char tile_x, const unsigned char tile_y, const unsigned char entity_type_flags, const signed char move_x, const signed char move_y) {
  static unsigned char state;
  static unsigned char digger;

  tile_index = TILE_INDEX(tile_x, tile_y);

  // Dig at soft tiles.
  tile_flags = tileset.flags[map.tile[tile_index]];
  if (entity_type_flags & ETF_DIGS && tile_flags & TILEF_SOFT) {
    digger = entities_spawn(E_DIGGER, tile_x, tile_y, 0, 0);
    state = 0;
    if (move_x < 0) {
      state = ST_LVL_DIG_H;
    } else if (move_x > 0) {
      state = ST_LVL_DIG_H;
      entities.flags[digger] = ENTITYF_FLIPX;
    } else if (move_y < 0) {
      state = ST_LVL_DIG_V;
    } else if (move_y > 0) {
      state = ST_LVL_DIG_V;
      entities.flags[digger] = ENTITYF_FLIPY;
    }
    if (state) {
      entities_set_state(digger, state);
      sfx_play_pan(SFX_LVL_DIG, 0x05, tile_x, tile_y);
    }

  // Handle special tiles.
  } else if (entity_type_flags & ETF_SPECIAL && tile_flags & TILEF_SPECIAL) {
    level_tile_execute_special(tile_x, tile_y);

  }
}

const unsigned char EXPLODE_X_CHANGE[] = {0, 1, 2, 0, 1, 2, 0, 1, 2};
const unsigned char EXPLODE_Y_CHANGE[] = {0, 0, 0, 1, 1, 1, 2, 2, 2};

void level_tile_start_explosion(unsigned char x, unsigned char y) {
  static unsigned char i;
  static unsigned char entity;
  static unsigned char lx;
  static unsigned char ly;

  sfx_play_pan(SFX_LVL_EXPLODE, 0x50, x, y);

  x -= 1;
  y -= 1;

  for (i = 0; i < 9; i++) {
    lx = x + EXPLODE_X_CHANGE[i];
    ly = y + EXPLODE_Y_CHANGE[i];

    tile_index = TILE_INDEX(lx, ly);
    tile = map.tile[tile_index];
    if (!tile || tileset.flags[tile] & TILEF_DESTRUCTIBLE) {
      entity = entities_spawn(E_ANIM, lx, ly, 0, 0);
      entities_set_state(entity, ST_LVL_EXPLODE);
    }
  }
}

void level_tile_explode(unsigned char x, unsigned char y) {
  static unsigned char entity;

  tile_index = TILE_INDEX(x, y);

  // Kill entities.
  entity = map.owner[tile_index];
  if (entity != 0xFF) {
    entities_explode(entity);
  }

  // Explosive tiles.
  tile = map.tile[tile_index];
  if (tile == T_LVL_TNT) {
    level_tile_start_explosion(x, y);
  }

  level_tile_clear(x, y);
}
