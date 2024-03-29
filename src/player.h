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

#define PLAYER_KILL_TIMEOUT 0
#define PLAYER_KILL_CRUSH   1
#define PLAYER_KILL_MONSTER 2
#define PLAYER_KILL_BURN    3

typedef struct player_info_t {
  unsigned char entity;
  unsigned char tnt;
} player_info_t;

extern player_info_t player;


void player_init(const unsigned char index);
void player_update(const unsigned char index);
void player_place_tnt(const unsigned char x, const unsigned char y);
void player_kill(const unsigned char index, const unsigned char method);

#endif
