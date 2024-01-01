#include <cx16.h>

#include "player.h"
#include "sprite_types.h"
#include "entity_types.h"
#include "entities.h"
#include "state_labels.h"
#include "input.h"
#include "level.h"
#include "main.h"
#include "tile_names.h"

// Local player data.
player_info_t player;

// Module shared variables.
static signed char x;
static signed char y;
static unsigned char tile_x;
static unsigned char tile_y;
static unsigned char flags;
static unsigned char state;
static unsigned char tile_flags;
static unsigned char local_state;
static unsigned char is_disabled;

void player_init(const unsigned char index) {
  entities.data[index] = STATE_IDLE;
  entities_set_state(index, ST_LVL_PLAYER_TAP);
}

void player_update(const unsigned char index) {
  x = entities.p_x[index];
  y = entities.p_y[index];
  tile_x = entities.tile_x[index];
  tile_y = entities.tile_y[index];
  state = entities.state[index];
  is_disabled = entities.data[index] & PLAYER_DATA_DISABLED;
  local_state = entities.data[index] & PLAYER_DATA_LOCAL_STATE;

  // Tile-to-tile movement.
  if (local_state == STATE_UP || local_state == STATE_DOWN) {
    if (!y) {
      local_state = STATE_IDLE;
    }
  } else if (local_state == STATE_LEFT || local_state == STATE_RIGHT || local_state == STATE_PUSH_RIGHT || local_state == STATE_PUSH_LEFT) {
    if (!x) {
      local_state = STATE_IDLE;
    }

  } else if (local_state == STATE_PUSH_LEFT_START) {

    // Cancel push.
    if (!(input1 & JOY_LEFT_MASK) && !x) {
      local_state = STATE_IDLE;

    // Start push if state advanced.
    } else if (state != ST_LVL_PLAYER_PUSH_RIGHT) {
      if (level_tile_push(tile_x - 1, tile_y, -1)) {
        entities_tile_move(index, -1, 0, 0);
        local_state = STATE_PUSH_LEFT;
      } else {
        local_state = STATE_IDLE;
      }
    }

  } else if (local_state == STATE_PUSH_RIGHT_START) {

    // Cancel push.
    if (!(input1 & JOY_RIGHT_MASK) && !x) {
      local_state = STATE_IDLE;

    // Start push if state advanced.
    } else if (state != ST_LVL_PLAYER_PUSH_RIGHT) {
      if (level_tile_push(tile_x + 1, tile_y, 1)) {
        entities_tile_move(index, 1, 0, 0);
        local_state = STATE_PUSH_RIGHT;
      } else {
        local_state = STATE_IDLE;
      }
    }

  // Walk into exit.
  } else if (local_state == STATE_EXIT) {
    if (state == ST_LVL_PLAYER_EXIT + 7) {
      game_action = GAMEACTION_LOAD_LEVEL;
      level_next = level_current + 1;
    }

  }

  // Player movement, if not already moving.
  if (local_state == STATE_IDLE && !is_disabled) {

    // Exit.
    if (map.tile[TILE_INDEX(tile_x, tile_y)] == T_LVL_EXIT_OPEN) {
      local_state = STATE_EXIT;
      is_disabled = PLAYER_DATA_DISABLED;

    // Touch nearby tile.
    } else if (input1 & JOY_BTN_A_MASK) {
      if (input1_change & JOY_UP_MASK && input1 & JOY_UP_MASK) {
        level_tile_touch(tile_x, tile_y - 1, entity_types.flags[E_PLAYER], 0, -1);
        level_tile_clear(tile_x, tile_y - 1);
      } else if (input1_change & JOY_DOWN_MASK && input1 & JOY_DOWN_MASK) {
        level_tile_touch(tile_x, tile_y + 1, entity_types.flags[E_PLAYER], 0, 1);
        level_tile_clear(tile_x, tile_y + 1);
      } else if (input1_change & JOY_LEFT_MASK && input1 & JOY_LEFT_MASK) {
        level_tile_touch(tile_x - 1, tile_y, entity_types.flags[E_PLAYER], -1, 0);
        level_tile_clear(tile_x - 1, tile_y);
      } else if (input1_change & JOY_RIGHT_MASK && input1 & JOY_RIGHT_MASK) {
        level_tile_touch(tile_x + 1, tile_y, entity_types.flags[E_PLAYER], 1, 0);
        level_tile_clear(tile_x + 1, tile_y);
      }

    // Move.
    } else {

      // Move up.
      if (input1 & JOY_UP_MASK) {
        if (!level_tile_is_blocked(tile_x, tile_y - 1)) {
          local_state = STATE_UP;
          entities_tile_move(index, 0, -1, 0);
        }

      // Move down.
      } else if (input1 & JOY_DOWN_MASK) {
        if (!level_tile_is_blocked(tile_x, tile_y + 1)) {
          local_state = STATE_DOWN;
          entities_tile_move(index, 0, 1, 0);
        }

      // Move left.
      } else if (input1 & JOY_LEFT_MASK) {
        tile_flags = level_tile_flags(tile_x - 1, tile_y);
        if (!level_tile_is_blocked(tile_x - 1, tile_y)) {
          local_state = STATE_LEFT;
          entities_tile_move(index, -1, 0, 0);

        } else if (tile_flags & TILEF_PUSHABLE) {
          local_state = STATE_PUSH_LEFT_START;
        }

      // Move right.
      } else if (input1 & JOY_RIGHT_MASK) {
        tile_flags = level_tile_flags(tile_x + 1, tile_y);
        if (!level_tile_is_blocked(tile_x + 1, tile_y)) {
          local_state = STATE_RIGHT;
          entities_tile_move(index, 1, 0, 0);

        } else if (tile_flags & TILEF_PUSHABLE) {
          local_state = STATE_PUSH_RIGHT_START;
        }
      }
    }
  }

  // Change to new entity state if our local state changed.
  if ((entities.data[index] & PLAYER_DATA_LOCAL_STATE) != local_state) {
    flags = 0;

    switch (local_state) {
      case STATE_IDLE: state = ST_LVL_PLAYER_TAP; break;

      case STATE_UP: state = ST_LVL_PLAYER_UP; break;
      case STATE_DOWN: state = ST_LVL_PLAYER_DOWN; break;
      case STATE_LEFT: state = ST_LVL_PLAYER_RIGHT; flags = ENTITYF_FLIPX; break;
      case STATE_RIGHT: state = ST_LVL_PLAYER_RIGHT; break;

      case STATE_PUSH_LEFT: state = ST_LVL_PLAYER_PUSH_RIGHT; flags = ENTITYF_FLIPX; break;
      case STATE_PUSH_RIGHT: state = ST_LVL_PLAYER_PUSH_RIGHT; break;

      case STATE_PUSH_LEFT_START: state = ST_LVL_PLAYER_PUSH_RIGHT; flags = ENTITYF_FLIPX; break;
      case STATE_PUSH_RIGHT_START: state = ST_LVL_PLAYER_PUSH_RIGHT; break;

      case STATE_EXIT: state = ST_LVL_PLAYER_EXIT; break;
    }

    entities.flags[index] = flags;
    entities_set_state(index, state);
  }

  entities.data[index] = local_state | is_disabled;
}
