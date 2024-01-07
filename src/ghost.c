#include "ghost.h"
#include "level.h"
#include "entities.h"
#include "state_labels.h"
#include "random.h"

const unsigned char DIRECTION_STATES[] = {
  ST_LVL_GHOST_UP,
  ST_LVL_GHOST_RIGHT,
  ST_LVL_GHOST_DOWN,
  ST_LVL_GHOST_RIGHT,
};

static unsigned char tile_x;
static unsigned char tile_y;
static unsigned char data;
static unsigned char state;
static unsigned char flags;
static signed char dir_x = 0;
static signed char dir_y = 0;
static unsigned int tile_index;

void ghost_init(const unsigned char index) {
  entities_set_state(index, ST_LVL_GHOST_DOWN);
  entities.counter[index] += RANDOM;
  entities.data[index] = RANDOM >> 6;
}

void ghost_update(const unsigned char index) {
  if (entities.p_x[index] || entities.p_y[index]) {
    return;
  }

  tile_x = entities.tile_x[index];
  tile_y = entities.tile_y[index];
  dir_x = 0;
  dir_y = 0;

  data = entities.data[index];
  if (data == GHOST_DIR_UP) {
    dir_y = -1;
  } else if (data == GHOST_DIR_RIGHT) {
    dir_x = 1;
  } else if (data == GHOST_DIR_DOWN) {
    dir_y = 1;
  } else if (data == GHOST_DIR_LEFT) {
    dir_x = -1;
  }

  tile_x += dir_x;
  tile_y += dir_y;

  tile_index = TILE_INDEX(tile_x, tile_y);
  if (map.tile[tile_index] || map.owner[tile_index] != 0xFF) {
    data = (data + 1) & 3;
  } else {
    entities_tile_move(index, dir_x, dir_y, TILE_MOVE_NO_EVALUATE);

    // Set state for this direction.
    if (data == GHOST_DIR_LEFT) {
      flags = ENTITYF_FLIPX;
    } else {
      flags = 0;
    }
    entities.flags[index] = flags;

    state = DIRECTION_STATES[data];
    entities_set_state(index, state);
  }

  entities.data[index] = data;
}

void ghost_destroy(const unsigned char index) {

}
