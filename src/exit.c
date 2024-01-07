#include <cx16.h>

#include "entities.h"
#include "level.h"
#include "exit.h"
#include "state_labels.h"

static unsigned char exits[EXIT_MAX];
static unsigned char exit_count = 0;

static unsigned char i;

void exit_init(const unsigned char index) {
  if (exit_count == EXIT_MAX) {
    entities_free(index);
    return;
  }

  // Freeze on invisible state.
  entities_set_state(index, ST_LVL_SPARK);
  entities.counter[index] = 0;

  // Put down closed exit tile.
  level_tile_set(TILE_INDEX(entities.tile_x[index], entities.tile_y[index]), 35);

  // Register as one of the exits.
  exits[exit_count] = index;
  exit_count++;
}

void exit_open() {
  unsigned char exit_index;

  for (i = 0; i < exit_count; i++) {
    exit_index = exits[i];
    level_tile_set(TILE_INDEX(entities.tile_x[exit_index], entities.tile_y[exit_index]), 34);
    entities_set_state(exit_index, ST_LVL_SPARK);
  }
}
