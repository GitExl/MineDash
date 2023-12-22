#include <cx16.h>

#include "entities.h"
#include "level.h"

void digger_init(unsigned char index) {
  entities.data[index] = 8;
}

void digger_update(unsigned char index) {
  if (!--entities.data[index]) {
    level_tile_clear(entities.tile_x[index], entities.tile_y[index]);
    entities_free(index);
  }
}
