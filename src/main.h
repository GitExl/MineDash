#ifndef MAIN_H
#define MAIN_H

#define GAMESTATE_LEVEL       0
#define GAMESTATE_INFO        1
#define GAMESTATE_PAUSE       2

#define GAMEACTION_NONE       0
#define GAMEACTION_LOAD_LEVEL 1
#define GAMEACTION_SHOW_INFO  2
#define GAMEACTION_PAUSE      4

extern unsigned char game_state;

extern unsigned char game_action;
extern unsigned char next_level;

#endif
