#include <cx16.h>

#include "entities.h"
#include "level.h"

void digger_destroy(const unsigned char index) {
  level_tile_clear(entities.tile_x[index], entities.tile_y[index]);
}
