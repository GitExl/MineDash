#include "ghost.h"
#include "entities.h"
#include "state_labels.h"
#include "random.h"

void ghost_init(const unsigned char index) {
  entities_set_state(index, ST_LVL_GHOST_DOWN);
  entities.counter[index] += RANDOM;
}

void ghost_update(const unsigned char index) {

}

void ghost_destroy(const unsigned char index) {

}
