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
static unsigned char tile_x;
static unsigned char tile_y;
static unsigned char local_type;
static unsigned char local_state;

void faller_init(unsigned char index) {
  local_type = entities.data[index] & 0x0F;
  local_state = entities.data[index] & 0xF0;

  if (local_state == FALLER_STATE_FALL) {
    entities_set_state(index, fall_states[local_type]);
  } else if (local_state == FALLER_STATE_ROLL) {
    entities_set_state(index, roll_states[local_type]);
  }
}

void faller_update(unsigned char index) {
  x = entities.p_x[index];
  y = entities.p_y[index];
  tile_x = entities.tile_x[index];
  tile_y = entities.tile_y[index];
  local_type = entities.data[index] & 0x0F;

  // Despawn and re-evaluate when at tile destination.
  if (!x && !y) {
    level_tile_set(tile_x, tile_y, tiles[local_type]);
    entities_free(index);
    level_gravity_evaluate(tile_x, tile_y, GF_ABOVE | GF_LEFT | GF_RIGHT);
  }
}
