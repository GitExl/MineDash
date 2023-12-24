#ifndef PLAYER_H
#define PLAYER_H

#define STATE_IDLE             0
#define STATE_UP               1
#define STATE_DOWN             2
#define STATE_LEFT             3
#define STATE_RIGHT            4
#define STATE_PUSH_LEFT_START  5
#define STATE_PUSH_LEFT        6
#define STATE_PUSH_RIGHT_START 7
#define STATE_PUSH_RIGHT       8
#define STATE_EXIT             9
#define STATE_CRUSH            10
#define STATE_BURN             11

#define PLAYER_DATA_DISABLED    0b00010000
#define PLAYER_DATA_LOCAL_STATE 0b00001111

void player_init(const unsigned char index);
void player_update(const unsigned char index);

#endif
