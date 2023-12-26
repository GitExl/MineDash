#include <cx16.h>

#include "faller.h"
#include "entities.h"
#include "level.h"
#include "state_labels.h"
#include "tile_names.h"

// States for falling types.
static unsigned char fall_states[] = {
  ST_LVL_ROCK,
  ST_LVL_GOLD,
  ST_LVL_DIAMOND,
};

// States for rolling types.
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

static signed char x;
static signed char y;
static unsigned int tile_index;

void faller_init(const unsigned char index) {
  const unsigned char tile_x = entities.tile_x[index];
  const unsigned char tile_y = entities.tile_y[index];
  const unsigned char local_type = (entities.data[index] & FALLER_DATA_TYPE) >> 2;
  const unsigned char local_state = entities.data[index] & FALLER_DATA_STATE;

  level_tile_set(tile_x, tile_y, 0);
  if (local_state == FALLER_STATE_FALL) {
    entities_set_state(index, fall_states[local_type]);
  } else {
    entities_set_state(index, roll_states[local_type]);
  }
}

void faller_update(const unsigned char index) {
  const unsigned char tile_x = entities.tile_x[index];
  const unsigned char tile_y = entities.tile_y[index];
  const unsigned char local_type = (entities.data[index] & FALLER_DATA_TYPE) >> 2;
  const unsigned char local_state = entities.data[index] & FALLER_DATA_STATE;

  signed char dir_x = 0;
  signed char dir_y = 0;
  unsigned char move_flags = 0;

  x = entities.p_x[index];
  y = entities.p_y[index];

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

    // Move to target position if the tile is still empty.
    tile_index = TILE_INDEX(tile_x + dir_x, tile_y + dir_y);
    if (!map.tile[tile_index] && map.owner[tile_index] == 0xFF) {
      entities_tile_move(index, dir_x, dir_y, move_flags);
      entities.data[index] = FALLER_STATE_IDLE | (local_type << 2);
    } else {
      level_tile_set(tile_x, tile_y, tiles[local_type]);
      entities_free(index);
      return;
    }

  // Despawn and re-evaluate when at tile destination.
  } else if (!x && !y) {
    level_tile_set(tile_x, tile_y, tiles[local_type]);
    entities_free(index);
    level_gravity_evaluate(tile_x, tile_y, GF_ABOVE | GF_LEFT | GF_RIGHT | GF_CRUSH);
    return;
  }
}
