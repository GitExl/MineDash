#include "ghost.h"
#include "level.h"
#include "entities.h"
#include "state_labels.h"
#include "entity_types.h"
#include "random.h"
#include "player.h"
#include "sfx.h"
#include "sfx_labels.h"

const unsigned char DIRECTION_STATES[] = {
  ST_LVL_GHOST_UP,
  ST_LVL_GHOST_RIGHT,
  ST_LVL_GHOST_DOWN,
  ST_LVL_GHOST_RIGHT,
};

static unsigned char tile_x;
static unsigned char tile_y;
static unsigned char dir;
static unsigned char state;
static unsigned char flags;
static unsigned char tile_owner;
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

  dir = entities.data[index];
  if (dir == GHOST_DIR_UP) {
    dir_y = -1;
  } else if (dir == GHOST_DIR_RIGHT) {
    dir_x = 1;
  } else if (dir == GHOST_DIR_DOWN) {
    dir_y = 1;
  } else if (dir == GHOST_DIR_LEFT) {
    dir_x = -1;
  }

  tile_x += dir_x;
  tile_y += dir_y;

  tile_index = TILE_INDEX(tile_x, tile_y);
  tile_owner = map.owner[tile_index];
  if (map.tile[tile_index] || tile_owner != 0xFF) {
    dir = (dir + 1) & 3;

    if (tile_owner != 0xFF) {
      if (entities.type[tile_owner] == E_PLAYER) {
        player_kill(tile_owner, PLAYER_KILL_MONSTER);
      }
    }

    if (dir == GHOST_DIR_LEFT) {
      flags = ENTITYF_FLIPX;
    } else {
      flags = 0;
    }
    entities.flags[index] = flags;

    state = DIRECTION_STATES[dir];
    entities_set_state(index, state);

  } else {
    entities_tile_move(index, dir_x, dir_y, TILE_MOVE_NO_EVALUATE);
    if (!RANDOM) {
      sfx_play_pan(SFX_LVL_GHOST, 0x10, tile_x, tile_y);
    }
  }

  entities.data[index] = dir;
}
