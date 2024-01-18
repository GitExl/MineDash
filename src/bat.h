#ifndef BAT_H
#define BAT_H

#define BAT_DIR_UP    0
#define BAT_DIR_RIGHT 1
#define BAT_DIR_DOWN  2
#define BAT_DIR_LEFT  3

void bat_init(const unsigned char index);
void bat_update(const unsigned char index);

#endif
