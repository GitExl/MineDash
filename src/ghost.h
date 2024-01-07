#ifndef GHOST_H
#define GHOST_H

#define GHOST_DIR_UP    0
#define GHOST_DIR_RIGHT 1
#define GHOST_DIR_DOWN  2
#define GHOST_DIR_LEFT  3

void ghost_init(const unsigned char index);
void ghost_update(const unsigned char index);
void ghost_destroy(const unsigned char index);

#endif
