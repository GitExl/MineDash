#include <cx16.h>

#include "entities.h"
#include "level.h"

void diamond_init(const unsigned char index) {
  entities.data[index] = 6;
}

void diamond_update(const unsigned char index) {
  if (!--entities.data[index]) {
    level_tile_clear(entities.tile_x[index], entities.tile_y[index]);
    entities_free(index);
  }
}
