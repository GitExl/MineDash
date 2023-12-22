#ifndef ROCK_H
#define ROCK_H

#define ROCK_STATE_IDLE  0
#define ROCK_STATE_DOWN  1
#define ROCK_STATE_LEFT  2
#define ROCK_STATE_RIGHT 3

void rock_init(unsigned char index);
void rock_update(unsigned char index);

#endif
