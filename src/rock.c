#include <cx16.h>

#include "rock.h"
#include "entities.h"
#include "level.h"
#include "state_labels.h"
#include "tile_names.h"

static signed char x;
static signed char y;
static unsigned char tile_x;
static unsigned char tile_y;
static unsigned char local_state;
static unsigned char tile_index;

void rock_init(unsigned char index) {
  entities_set_state(index, ST_LVL_ROCK);
}

void rock_update(unsigned char index) {
  x = entities.p_x[index];
  y = entities.p_y[index];
  tile_x = entities.tile_x[index];
  tile_y = entities.tile_y[index];
  local_state = entities.data[index];

  if (local_state == ROCK_STATE_DOWN) {
    if (!y) {
      local_state = ROCK_STATE_IDLE;

      // Stop falling.
      level_tile_set(tile_x, tile_y, T_LVL_ROCK);
      tile_index = TILE_INDEX(tile_x, tile_y);
      level_owner[tile_index] = 0;
      entities_free(index);

      // Re-evaluate at new position.
      level_gravity_evaluate(tile_x, tile_y, GF_ABOVE | GF_LEFT | GF_RIGHT);

      return;
    }
  }

  entities.data[index] = local_state;
}
