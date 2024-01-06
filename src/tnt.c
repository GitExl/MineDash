#include <cx16.h>

#include "tnt.h"
#include "entities.h"
#include "level.h"
#include "sfx.h"
#include "sfx_labels.h"
#include "state_labels.h"

void tnt_init(const unsigned char index) {
  entities_set_state(index, ST_LVL_TNT);
  entities.data[index] = 0;
}

void tnt_update(const unsigned char index) {
  if (!entities.data[index]--) {
    sfx_play_pan(SFX_LVL_TNT_TICK, 0x80, entities.tile_x[index], entities.tile_y[index]);
    entities.data[index] = 24;
  }
}

void tnt_destroy(const unsigned char index) {
  level_tile_start_explosion(entities.tile_x[index], entities.tile_y[index]);
}
