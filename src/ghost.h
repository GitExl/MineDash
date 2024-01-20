#ifndef GHOST_H
#define GHOST_H

#define GHOST_DIR_UP     0
#define GHOST_DIR_RIGHT  1
#define GHOST_DIR_DOWN   2
#define GHOST_DIR_LEFT   3

#define GHOST_DATA_DIR       0b01100000
#define GHOST_DATA_DIR_SHIFT 5

#define GHOST_DATA_TIMER 0b00011111

void ghost_init(const unsigned char index);
void ghost_update(const unsigned char index);

#endif
