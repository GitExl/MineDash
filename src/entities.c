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

#include "player.h"
#include "digger.h"
#include "gold.h"
#include "diamond.h"
#include "exit.h"
#include "explode.h"
#include "rock.h"

// Entities.
entity_t entities;

// Entity states.
entity_states_t entity_states;

// Player entity index.
unsigned char entity_player = 0;

// The next (probably) unused entity index.
unsigned char next_unused = 0;

static unsigned char i;
static int vera_x;
static int vera_y;
static unsigned char flags;
static unsigned char type;
static unsigned char state;
static unsigned char counter;
static unsigned int address;

static unsigned char ret_property_mask;

void entities_free(const unsigned char index) {

  // Disable VERA sprite.
  VERA.address = 0xFC06 + index * 8;
  VERA.address_hi = 0x01 | VERA_INC_1;
  VERA.data0 = 0;

  entities.flags[index] |= ENTITYF_UNUSED;
  if (index < next_unused) {
    next_unused = index;
  }
}

void entities_update_vera_sam() {
  VERA.address_hi = 0x01 | VERA_INC_1;

  for (i = 0; i < ENTITY_MAX; i++) {
    flags = entities.flags[i];
    if (flags & ENTITYF_UNUSED) {
      continue;
    }

    vera_x = (entities.tile_x[i] << 4) + entities.p_x[i] - camerax;
    vera_y = (entities.tile_y[i] << 4) + entities.p_y[i] - cameray;

    // Only update coordinates, skip 2 bytes of address.
    address = 0xFC02 + i * 8;
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

void entities_set_state(const unsigned char entity, const unsigned char state) {
  unsigned char sprite = entity_states.sprite[state];

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
  unsigned char entity_flags = entities.flags[entity];
  unsigned char state_flags = entity_states.flags[state_index];

  ret_property_mask = 0b00000000;
  if (entity_flags & ENTITYF_FLIPX) {
    ret_property_mask |= 0b00000001;
  }
  if (entity_flags & ENTITYF_FLIPY) {
    ret_property_mask |= 0b00000010;
  }
  if (!(entity_flags & ENTITYF_INVISIBLE) && !(state_flags & STATEF_INVISIBLE)) {
    ret_property_mask |= 0b00001000;
  }
}

void entities_tile_move(const unsigned char entity, const signed char move_x, const signed char move_y) {
  static unsigned char tile_x;
  static unsigned char tile_y;
  static signed char x;
  static signed char y;
  static unsigned int tile_index;
  static unsigned char tile_flags;
  static unsigned char digger;

  x = entities.p_x[entity];
  y = entities.p_y[entity];
  tile_x = entities.tile_x[entity];
  tile_y = entities.tile_y[entity];

  // Clear out current tile ownership.
  tile_index = TILE_INDEX(tile_x, tile_y);
  level_owner[tile_index] = 0;

  // Move to new tile.
  tile_x += move_x;
  tile_y += move_y;
  x = -(move_x << 4);
  y = -(move_y << 4);

  // Take ownership of new tile.
  tile_index = TILE_INDEX(tile_x, tile_y);
  level_owner[tile_index] = entity;

  // Act on destination tile.
  tile_flags = tileset.flags[level_tile[tile_index]];
  if (tile_flags & TILEF_SOFT) {
    digger = entities_spawn(E_DIGGER, tile_x, tile_y);
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
    entities_set_state(digger, state);

  } else if (tile_flags & TILEF_SPECIAL) {
    level_tile_special(tile_x, tile_y);

  }

  entities.tile_x[entity] = tile_x;
  entities.tile_y[entity] = tile_y;
  entities.p_x[entity] = x;
  entities.p_y[entity] = y;
}

void entities_update() {
  for (i = 0; i < ENTITY_MAX; i++) {
    flags = entities.flags[i];
    if (flags & ENTITYF_UNUSED) {
      continue;
    }

    // Advance state.
    counter = entities.counter[i];
    state = entities.state[i];
    if (counter) {
      --counter;
      if (!counter) {

        // Repeat state 50% of the time.
        if (!(entity_states.flags[state] & STATEF_RANDOM_REPEAT && (RANDOM & 0x80))) {
          state = entity_states.next[state];
        }
        entities_set_state(i, state);
      } else {
        entities.counter[i] = counter;
      }
    }

    // Always move pixel position to 0,0.
    if (entities.p_x[i] < 0) {
      entities.p_x[i] += 2;
    } else if (entities.p_x[i] > 0) {
      entities.p_x[i] -= 2;
    }
    if (entities.p_y[i] < 0) {
      entities.p_y[i] += 2;
    } else if (entities.p_y[i] > 0) {
      entities.p_y[i] -= 2;
    }

    // Run update functions.
    type = entities.type[i];
    switch (type) {
      case E_PLAYER: player_update(i); break;
      case E_DIGGER: digger_update(i); break;
      case E_GOLD: gold_update(i); break;
      case E_DIAMOND: diamond_update(i); break;
      case E_EXPLODE: explode_update(i); break;
      case E_ROCK: rock_update(i); break;
    }
  }
}

void entities_load(const char* entity_filename) {

  // Load level entities.
  cbm_k_setnam(entity_filename);
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(0, (unsigned int)&entities);

  for (i = 0; i < ENTITY_MAX; i++) {
    flags = entities.flags[i];
    if (flags & ENTITYF_UNUSED) {
      continue;
    }

    entities_init_entity(i, entities.type[i]);
  }
}

unsigned char entities_spawn(const unsigned char type, const unsigned char tile_x, const unsigned char tile_y) {

  // Find the first unused entity.
  for (i = next_unused; i < ENTITY_MAX; i++) {
    if (entities.flags[i] & ENTITYF_UNUSED) {
      break;
    }
  }

  entities.type[i] = type;
  entities.flags[i] = 0;
  entities.p_x[i] = 0;
  entities.p_y[i] = 0;
  entities.tile_x[i] = tile_x;
  entities.tile_y[i] = tile_y;

  entities_init_entity(i, type);

  return i;
}

void entities_init_entity(const char index, const char type) {
  switch (type) {
    case E_PLAYER: player_init(index); break;
    case E_DIGGER: digger_init(index); break;
    case E_GOLD: gold_init(index); break;
    case E_DIAMOND: diamond_init(index); break;
    case E_EXIT: exit_init(index); break;
    case E_EXPLODE: explode_init(index); break;
    case E_ROCK: rock_init(index); break;
  }
}

void entities_load_states(const char* states_filename) {

  // Level states, 1k
  cbm_k_setnam(states_filename);
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(0, (unsigned int)&entity_states);
}
