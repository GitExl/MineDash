#include <cx16.h>

#include "tnt.h"
#include "entities.h"
#include "level.h"
#include "state_labels.h"

void tnt_init(const unsigned char index) {
  entities_set_state(index, ST_LVL_TNT);
}

void tnt_update(const unsigned char index) {
  // fizzle/tick sound step
}

void tnt_destroy(const unsigned char index) {
  level_tile_start_explosion(entities.tile_x[index], entities.tile_y[index]);
}
