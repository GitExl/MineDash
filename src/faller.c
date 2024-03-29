#include <cx16.h>

#include "faller.h"
#include "entities.h"
#include "level.h"
#include "state_labels.h"
#include "tile_names.h"
#include "sfx.h"
#include "sfx_labels.h"

// States for falling.
static unsigned char fall_states[] = {
  ST_LVL_ROCK,
  ST_LVL_GOLD,
  ST_LVL_DIAMOND,
};

// States for rolling.
static unsigned char roll_states[] = {
  ST_LVL_ROCK_ROLL,
  ST_LVL_GOLD_ROLL,
  ST_LVL_DIAMOND_ROLL,
};

// Stationary tiles.
static unsigned char tiles[] = {
  T_LVL_ROCK,
  T_LVL_GOLD,
  T_LVL_DIAMOND,
};

// Faller hit SFX.
static unsigned char hit_sfx[] = {
  SFX_LVL_ROCK_HIT,
  SFX_LVL_GOLD_HIT,
  SFX_LVL_GOLD_HIT,
};

static signed char x;
static signed char y;
static signed char state;
static unsigned int tile_index;

void faller_init(const unsigned char index) {
  static unsigned char tile_x;
  static unsigned char tile_y;
  static unsigned char local_type;

  tile_x = entities.tile_x[index];
  tile_y = entities.tile_y[index];
  local_type = (entities.data[index] & FALLER_DATA_TYPE) >> FALLER_DATA_TYPE_SHIFT;

  entities_set_state(index, fall_states[local_type]);
  level_tile_set(TILE_INDEX(tile_x, tile_y), 0);
}

void faller_update(const unsigned char index) {
  static unsigned char tile_x;
  static unsigned char tile_y;
  static unsigned char tile;
  static unsigned char local_type;
  static unsigned char local_state;
  static unsigned char sfx_index;
  static unsigned char tile_flags;

  unsigned char delay = entities.data[index] & FALLER_DATA_DELAY;
  signed char dir_x = 0;
  signed char dir_y = 0;
  unsigned char move_flags = 0;

  tile_x = entities.tile_x[index];
  tile_y = entities.tile_y[index];
  local_type = (entities.data[index] & FALLER_DATA_TYPE) >> FALLER_DATA_TYPE_SHIFT;
  local_state = entities.data[index] & FALLER_DATA_STATE;

  if (delay) {
    --delay;
  } else {

    // Re-evaluate when at tile destination.
    x = entities.p_x[index];
    y = entities.p_y[index];
    if (local_state == FALLER_STATE_IDLE && !x && !y) {
      tile_index = TILE_INDEX(tile_x, tile_y);
      tile = map.tile[tile_index];
      tile_flags = tileset.flags[tile];

      // Finished sinking into lava?
      if (tile_flags & TILEF_LETHAL) {
        entities_free(index);
        return;
      }

      state = level_tile_evaluate_gravity(tile_index, GF_ABOVE | GF_LEFT | GF_RIGHT | GF_CRUSH);
      sfx_index = hit_sfx[local_type];
      if (state) {
        local_state = state;
        faller_set_state(index, local_state, local_type);
        if (state != FALLER_STATE_FALL) {
          sfx_play_pan(sfx_index, 0x10, tile_x, tile_y);
        }

      // Stop moving, turn back into a tile.
      } else {
        sfx_play_pan(sfx_index, 0x10, tile_x, tile_y);
        level_tile_set(tile_index, tiles[local_type]);
        entities_free(index);
        return;

      }
    }

    // Perform actual roll or fall.
    if (local_state != FALLER_STATE_IDLE) {
      if (local_state == FALLER_STATE_ROLL_LEFT) {
        dir_x = -1;
      } else if (local_state == FALLER_STATE_PUSH_LEFT) {
        dir_x = -1;
        move_flags = TILE_MOVE_NO_EVALUATE;
      } else if (local_state == FALLER_STATE_ROLL_RIGHT) {
        dir_x = 1;
      } else if (local_state == FALLER_STATE_PUSH_RIGHT) {
        dir_x = 1;
        move_flags = TILE_MOVE_NO_EVALUATE;
      } else if (local_state == FALLER_STATE_FALL) {
        dir_y = 1;
      }

      tile_index = TILE_INDEX(tile_x + dir_x, tile_y + dir_y);
      tile = map.tile[tile_index];
      tile_flags = tileset.flags[tile];

      // Stop if the new tile is not empty.
      if (!(tile_flags & TILEF_LETHAL) && (tile || map.owner[tile_index] != 0xFF)) {
        tile_index = TILE_INDEX(tile_x, tile_y);
        level_tile_set(tile_index, tiles[local_type]);
        entities_free(index);
        return;
      }

      // Move to target position.
      entities_tile_move(index, dir_x, dir_y, move_flags);
      faller_set_state(index, local_state, local_type);
      local_state = FALLER_STATE_IDLE;

      // Sink into lava.
      if (tile_flags & TILEF_LETHAL) {
        sfx_play_pan(SFX_LVL_SINK, 0x20, tile_x, tile_y);
        entities_set_behind_map(index);
      }
    }
  }

  entities.data[index] = local_state | (local_type << FALLER_DATA_TYPE_SHIFT) | delay;
}

unsigned char faller_type_for_tile(const unsigned char tile) {
  switch (tile) {
    case T_LVL_ROCK: return FALLER_TYPE_ROCK;
    case T_LVL_GOLD: return FALLER_TYPE_GOLD;
    case T_LVL_DIAMOND: return FALLER_TYPE_DIAMOND;
  }

  return FALLER_TYPE_ROCK;
}

void faller_set_state(const unsigned char index, const unsigned char local_state, const unsigned char local_type_index) {
  entities_set_state(index, (local_state == FALLER_STATE_FALL) ? fall_states[local_type_index] : roll_states[local_type_index]);
  entities.flags[index] = (local_state == FALLER_STATE_ROLL_LEFT || local_state == FALLER_STATE_PUSH_LEFT) ? ENTITYF_FLIPX : 0;
}
