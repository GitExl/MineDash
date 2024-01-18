#include <cx16.h>
#include <cbm.h>

#include "camera.h"
#include "entities.h"
#include "sprites.h"
#include "entity_types.h"
#include "sprite_types.h"
#include "random.h"
#include "level.h"
#include "state_labels.h"
#include "sfx.h"
#include "sfx_labels.h"

#include "player.h"
#include "digger.h"
#include "exit.h"
#include "faller.h"
#include "tnt.h"
#include "ghost.h"
#include "bat.h"

// Entities.
entity_t entities;

// Entity states.
entity_states_t entity_states;

// Entity type information.
entity_type_t entity_types;

// Player entity index.
unsigned char entity_player = 0;

// The next (probably) unused entity index.
unsigned char next_unused = 0;

static int vera_x;
static int vera_y;
static unsigned char flags;
static unsigned char type;
static unsigned char state;
static unsigned char counter;
static unsigned int address;

static unsigned char ret_property_mask;

unsigned char entities_spawn(const unsigned char type, const unsigned char tile_x, const unsigned char tile_y, const unsigned char flags, const unsigned char data) {
  register unsigned char j;
  static unsigned int tile_index;

  // Find the first unused entity.
  for (j = next_unused; j < ENTITY_MAX; j++) {
    if (entities.flags[j] & ENTITYF_UNUSED) {
      break;
    }
  }

  entities.type[j] = type;
  entities.flags[j] = flags;
  entities.data[j] = data;
  entities.p_x[j] = 0;
  entities.p_y[j] = 0;
  entities.tile_x[j] = tile_x;
  entities.tile_y[j] = tile_y;

  // Set VERA sprite position right now.
  vera_x = (entities.tile_x[j] << 4) - camerax;
  vera_y = (entities.tile_y[j] << 4) - cameray;

  // Clear VERA sprite.
  address = 0xFC02 + j * 8;
  VERA.address = address;
  VERA.address_hi = 0x01 | VERA_INC_1;
  VERA.data0 = vera_x;
  VERA.data0 = vera_x >> 8;
  VERA.data0 = vera_y;
  VERA.data0 = vera_y >> 8;
  VERA.data0 = 0;
  entities_init_entity(j, type);

  if (entity_types.flags[type] & ETF_OWNERSHIP) {
    tile_index = TILE_INDEX(tile_x, tile_y);
    map.owner[tile_index] = j;
  }

  return j;
}

void entities_free(const unsigned char index) {
  static unsigned int tile_index;

  type = entities.type[index];
  switch (type) {
    case E_DIGGER: digger_destroy(index); break;
    case E_TNT: tnt_destroy(index); break;
  }

  // Disable VERA sprite.
  VERA.address = 0xFC06 + index * 8;
  VERA.address_hi = 0x01 | VERA_INC_1;
  VERA.data0 = 0;

  entities.flags[index] |= ENTITYF_UNUSED;
  if (index < next_unused) {
    next_unused = index;
  }

  if (entity_types.flags[type] & ETF_OWNERSHIP) {
    tile_index = TILE_INDEX(entities.tile_x[index], entities.tile_y[index]);
    map.owner[tile_index] = 0xFF;
  }
}

void entities_update_vera_sam() {
  register unsigned char j;

  VERA.address_hi = 0x01 | VERA_INC_1;

  for (j = 0; j < ENTITY_MAX; j++) {
    flags = entities.flags[j];
    if (flags & ENTITYF_UNUSED) {
      continue;
    }

    vera_x = (entities.tile_x[j] << 4) + entities.p_x[j] - camerax;
    vera_y = (entities.tile_y[j] << 4) + entities.p_y[j] - cameray;
    // vera_x = (entities.tile_x[j] << 4) - camerax;
    // vera_y = (entities.tile_y[j] << 4) - cameray;

    // Only update coordinates, skip 2 bytes of address.
    address = 0xFC02 + j * 8;
    VERA.address = address;
    VERA.data0 = vera_x;
    VERA.data0 = vera_x >> 8;
    VERA.data0 = vera_y;
    VERA.data0 = vera_y >> 8;
  }
}

void entities_set_invisible(const unsigned char entity) {
  entities.flags[entity] |= ENTITYF_INVISIBLE;

  entity_get_property_mask(entity, entities.state[entity]);

  VERA.address = 0xFC06 + entity * 8;
  VERA.address_hi = 0x01 | VERA_INC_1;
  VERA.data0 = ret_property_mask;
}

void entities_set_behind_map(const unsigned char entity) {
  entities.flags[entity] |= ENTITYF_BEHIND_MAP;

  entity_get_property_mask(entity, entities.state[entity]);

  VERA.address = 0xFC06 + entity * 8;
  VERA.address_hi = 0x01 | VERA_INC_1;
  VERA.data0 = ret_property_mask;
}

void entities_set_state(const unsigned char entity, const unsigned char state) {
  const unsigned char sprite = entity_states.sprite[state];

  address = sprites.address[sprite];
  entities.state[entity] = state;
  entities.counter[entity] = entity_states.duration[state];

  entity_get_property_mask(entity, state);

  VERA.address = 0xFC00 + entity * 8;
  VERA.address_hi = 0x01 | VERA_INC_1;
  VERA.data0 = address;
  VERA.data0 = address >> 8;

  VERA.address += 4;
  VERA.data0 = ret_property_mask;
  VERA.data0 = sprites.size_palette[sprite];
}

void entity_get_property_mask(const unsigned char entity, const unsigned char state_index) {
  const unsigned char entity_flags = entities.flags[entity];
  const unsigned char state_flags = entity_states.flags[state_index];

  ret_property_mask = 0b00000000;
  if (entity_flags & ENTITYF_FLIPX) {
    ret_property_mask |= 0b00000001;
  }
  if (entity_flags & ENTITYF_FLIPY) {
    ret_property_mask |= 0b00000010;
  }
  if (!(entity_flags & ENTITYF_INVISIBLE) && !(state_flags & STATEF_INVISIBLE)) {
    if (entity_flags & ENTITYF_BEHIND_MAP) {
      ret_property_mask |= 0b00000100;
    } else {
      ret_property_mask |= 0b00001000;
    }
  }
}

void entities_tile_move(const unsigned char entity, const signed char move_x, const signed char move_y, const unsigned char move_flags) {
  static unsigned char type;
  static unsigned char type_flags;
  static unsigned char entity_flags;

  static unsigned char tile_x;
  static unsigned char tile_y;
  static unsigned char dest_tile_x;
  static unsigned char dest_tile_y;

  static unsigned int tile_index;
  static unsigned int dest_tile_index;

  type = entities.type[entity];
  type_flags = entity_types.flags[type];
  tile_x = entities.tile_x[entity];
  tile_y = entities.tile_y[entity];

  tile_index = TILE_INDEX(tile_x, tile_y);

  dest_tile_x = tile_x + move_x;
  dest_tile_y = tile_y + move_y;

  // Move tile ownership.
  if (type_flags & ETF_OWNERSHIP) {
    map.owner[tile_index] = 0xFF;
    dest_tile_index = TILE_INDEX(dest_tile_x, dest_tile_y);
    map.owner[dest_tile_index] = entity;
  }

  level_tile_touch(dest_tile_x, dest_tile_y, type_flags, move_x, move_y);

  // Move to new position.
  entities.p_x[entity] = -(move_x << 4);
  entities.p_y[entity] = -(move_y << 4);
  entities.tile_x[entity] = dest_tile_x;
  entities.tile_y[entity] = dest_tile_y;

  entity_flags = entities.flags[entity];
  if (!(entity_flags & ENTITYF_BEHIND_MAP)) {
    if (move_flags & TILE_MOVE_NO_EVALUATE) {
      level_tile_set(tile_index, 0);
    } else {
      level_tile_clear(tile_x, tile_y);
    }
  }
}

void entities_update() {
  register unsigned char j;
  static unsigned char state_flags;

  for (j = 0; j < ENTITY_MAX; j++) {
    flags = entities.flags[j];
    if (flags & ENTITYF_UNUSED) {
      continue;
    }

    // Advance state.
    counter = entities.counter[j];
    state = entities.state[j];
    if (counter) {
      --counter;
      if (!counter) {
        state_flags = entity_states.flags[state];

        // Destroy.
        if (state_flags & STATEF_DESTROY) {
          entities_free(j);
          continue;
        }

        // Explode tile.
        if (state_flags & STATEF_EXPLODE_TILE) {
          level_tile_explode(entities.tile_x[j], entities.tile_y[j]);
        }

        // Repeat state 50% of the time.
        if (!(state_flags & STATEF_RANDOM_REPEAT && (RANDOM & 0x80))) {
          state = entity_states.next[state];
        }

        entities_set_state(j, state);
      } else {
        entities.counter[j] = counter;
      }
    }

    // Always move pixel position towards 0,0.
    if (entities.p_x[j] < 0) {
      entities.p_x[j] += 2;
    } else if (entities.p_x[j] > 0) {
      entities.p_x[j] -= 2;
    }
    if (entities.p_y[j] < 0) {
      entities.p_y[j] += 2;
    } else if (entities.p_y[j] > 0) {
      entities.p_y[j] -= 2;
    }

    // Run update functions.
    type = entities.type[j];
    switch (type) {
      case E_PLAYER: player_update(j); break;
      case E_FALLER: faller_update(j); break;
      case E_TNT: tnt_update(j); break;
      case E_GHOST: ghost_update(j); break;
      case E_BAT: bat_update(j); break;
    }
  }
}

void entities_load(const char* entity_filename) {
  register unsigned char j;

  // Load level entities.
  cbm_k_setnam(entity_filename);
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(0, (unsigned int)&entities);

  // Configure entity types.
  entity_types.flags[E_PLAYER] = ETF_OWNERSHIP | ETF_CRUSHABLE | ETF_DIGS | ETF_SPECIAL;
  entity_types.flags[E_FALLER] = ETF_OWNERSHIP;
  entity_types.flags[E_TNT] = ETF_OWNERSHIP | ETF_CRUSHABLE;
  entity_types.flags[E_GHOST] = ETF_OWNERSHIP | ETF_CRUSHABLE;
  entity_types.flags[E_BAT] = ETF_OWNERSHIP | ETF_CRUSHABLE;

  for (j = 0; j < ENTITY_MAX; j++) {
    flags = entities.flags[j];
    if (flags & ENTITYF_UNUSED) {
      continue;
    }

    entities_init_entity(j, entities.type[j]);
  }
}

void entities_init_entity(const char index, const char type) {
  switch (type) {
    case E_PLAYER: player_init(index); break;
    case E_EXIT: exit_init(index); break;
    case E_FALLER: faller_init(index); break;
    case E_TNT: tnt_init(index); break;
    case E_GHOST: ghost_init(index); break;
    case E_BAT: bat_init(index); break;
  }
}

void entities_load_states(const char* states_filename) {

  // Level states, 1k
  cbm_k_setnam(states_filename);
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(0, (unsigned int)&entity_states);
}

void entities_crush(const unsigned char entity) {
  static unsigned char type;
  static unsigned int tile_index;
  static unsigned char tile_x;
  static unsigned char tile_y;

  tile_x = entities.tile_x[entity];
  tile_y = entities.tile_y[entity];

  type = entities.type[entity];
  if (entity_types.flags[type] & ETF_CRUSHABLE) {
    switch (type) {
      case E_PLAYER:
        player_kill(entity, PLAYER_KILL_CRUSH);
        break;

      case E_TNT:
        level_tile_start_explosion(tile_x, tile_y);
        entities_free(entity);
        break;

      case E_GHOST:
        level_tile_start_explosion(tile_x, tile_y);
        entities_free(entity);
        break;

      case E_BAT:
        level_tile_start_explosion(tile_x, tile_y);
        entities_free(entity);
        break;
    }

    tile_index = TILE_INDEX(tile_x, tile_y);
    map.owner[tile_index] = 0xFF;
  }
}

void entities_explode(const unsigned char entity) {
  static unsigned char type;
  static unsigned int tile_index;
  static unsigned char tile_x;
  static unsigned char tile_y;

  tile_x = entities.tile_x[entity];
  tile_y = entities.tile_y[entity];

  type = entities.type[entity];
  if (entity_types.flags[type] & ETF_CRUSHABLE) {
    switch (type) {
      case E_PLAYER:
        player_kill(entity, PLAYER_KILL_BURN);
        break;

      case E_TNT:
        level_tile_start_explosion(tile_x, tile_y);
        break;
    }

    tile_index = TILE_INDEX(tile_x, tile_y);
    map.owner[tile_index] = 0xFF;
  }
}
