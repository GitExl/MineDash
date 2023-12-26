#include <cx16.h>

#include "entities.h"
#include "level.h"
#include "state_labels.h"

void explode_init(const unsigned char index) {
  entities_set_state(index, ST_LVL_EXPLODE);
  entities.data[index] = 21;
}

void explode_update(const unsigned char index) {
  if (!--entities.data[index]) {
    entities_free(index);
  }
}
