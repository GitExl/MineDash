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
#include "sfx.h"
#include "sfx_labels.h"
#include "text.h"

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
  static unsigned int tile_index;
  static unsigned char map_tile;
  static unsigned char entity_flags;

  static signed char move_x = 0;
  static signed char move_y = 0;
  static unsigned char move_tile_x = 0;
  static unsigned char move_tile_y = 0;
  static unsigned char move_state = 0;
  static unsigned char move_state_push = 0;
  static unsigned char tile = 0;
  static unsigned int move_tile_index = 0;

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
    tile_index = TILE_INDEX(tile_x, tile_y);
    map_tile = map.tile[tile_index];

    // Exit.
    if (map_tile == T_LVL_EXIT_OPEN) {
      local_state = STATE_EXIT;
      is_disabled = PLAYER_DATA_DISABLED;

    // Touch nearby tile by pressing A + Direction.
    } else if (input2 & JOY_BTN_A_MASK) {
      entity_flags = entities.flags[index];
      if (input1_change & JOY_UP_MASK && input1 & JOY_UP_MASK) {
        level_tile_touch(tile_x, tile_y - 1, entity_flags, 0, -1);
      } else if (input1_change & JOY_DOWN_MASK && input1 & JOY_DOWN_MASK) {
        level_tile_touch(tile_x, tile_y + 1, entity_flags, 0, 1);
      } else if (input1_change & JOY_LEFT_MASK && input1 & JOY_LEFT_MASK) {
        level_tile_touch(tile_x - 1, tile_y, entity_flags, -1, 0);
      } else if (input1_change & JOY_RIGHT_MASK && input1 & JOY_RIGHT_MASK) {
        level_tile_touch(tile_x + 1, tile_y, entity_flags, 1, 0);
      }

    // Drop TNT by pressing B + Direction.
    } else if (input1 & JOY_BTN_B_MASK) {
      if (input1_change & JOY_UP_MASK && input1 & JOY_UP_MASK) {
        player_place_tnt(tile_x, tile_y - 1);
      } else if (input1_change & JOY_DOWN_MASK && input1 & JOY_DOWN_MASK) {
        player_place_tnt(tile_x, tile_y + 1);
      } else if (input1_change & JOY_LEFT_MASK && input1 & JOY_LEFT_MASK) {
        player_place_tnt(tile_x - 1, tile_y);
      } else if (input1_change & JOY_RIGHT_MASK && input1 & JOY_RIGHT_MASK) {
        player_place_tnt(tile_x + 1, tile_y);
      }

    // Move or push when D-pad is down.
    } else {
      move_x = 0;
      move_y = 0;
      move_state = 0;
      move_state_push = 0;

      if (input1 & JOY_UP_MASK) {
        move_y = -1;
        move_state = STATE_UP;
      } else if (input1 & JOY_DOWN_MASK) {
        move_y = 1;
        move_state = STATE_DOWN;
      } else if (input1 & JOY_LEFT_MASK) {
        move_x = -1;
        move_state = STATE_LEFT;
        move_state_push = STATE_PUSH_LEFT_START;
      } else if (input1 & JOY_RIGHT_MASK) {
        move_x = 1;
        move_state = STATE_RIGHT;
        move_state_push = STATE_PUSH_RIGHT_START;
      }

      if (move_state) {
        move_tile_x = tile_x + move_x;
        move_tile_y = tile_y + move_y;

        move_tile_index = TILE_INDEX(move_tile_x, move_tile_y);
        tile = map.tile[move_tile_index];
        tile_flags = tileset.flags[tile];
        if (tile_flags & TILEF_LETHAL) {
          player_kill(index, PLAYER_KILL_BURN);
          return;
        }
        if (!((tile_flags & TILEF_BLOCKS) || (map.owner[move_tile_index] != 0xFF))) {
          local_state = move_state;
          entities_tile_move(index, move_x, move_y, 0);
        } else if (move_state_push && tile_flags & TILEF_PUSHABLE) {
          local_state = move_state_push;
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

void player_place_tnt(const unsigned char x, const unsigned char y) {
  static unsigned int tile_index;
  static unsigned char map_tile;
  static unsigned char map_owner;

  if (!player.tnt) {
    return;
  }

  // Tile must be empty.
  tile_index = TILE_INDEX(x, y);
  map_owner = map.owner[tile_index];
  if (map_owner != 0xFF) {
    return;
  }
  map_tile = map.tile[tile_index];
  if (map_tile) {
    return;
  }

  --player.tnt;
  level_hud_update = 1;
  entities_spawn(E_TNT, x, y, 0, 0);
}

void player_kill(const unsigned char index, const unsigned char method) {
  static unsigned char sfx;
  static unsigned char data;
  static unsigned char invisible;
  static unsigned int tile_index;

  if (entities.data[index] & PLAYER_DATA_DISABLED) {
    return;
  }

  data = PLAYER_DATA_DISABLED;

  switch (method) {
    case PLAYER_KILL_TIMEOUT:
      sfx = SFX_LVL_EXPLODE;
      invisible = 1;
      entities_set_state(entities_spawn(E_ANIM, entities.tile_x[entity_player], entities.tile_y[entity_player], 0, 0), ST_LVL_EXPLODE);
      break;

    case PLAYER_KILL_CRUSH:
      sfx = SFX_LVL_CRUSH;
      entities_set_state(index, ST_LVL_PLAYER_CRUSH);
      data |= STATE_CRUSH;
      break;

    case PLAYER_KILL_MONSTER:
      sfx = SFX_LVL_DEATH;
      invisible = 1;
      level_tile_start_explosion(tile_x, tile_y);
      break;

    case PLAYER_KILL_BURN:
      sfx = SFX_LVL_DEATH;
      entities_set_state(index, ST_LVL_PLAYER_BURN);
      data |= STATE_CRUSH;
      break;
  }

  entities.data[index] = data;
  if (invisible) {
    entities_set_invisible(index);
  }
  sfx_play_pan(sfx, 0x40, tile_x, tile_y);

  tile_index = TILE_INDEX(entities.tile_x[index], entities.tile_y[index]);
  map.owner[tile_index] = 0xFF;
}
