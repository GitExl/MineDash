#ifndef FALLER_H
#define FALLER_H

#define FALLER_TYPE_ROCK    0x00
#define FALLER_TYPE_GOLD    0x01
#define FALLER_TYPE_DIAMOND 0x02

#define FALLER_STATE_FALL 0x10
#define FALLER_STATE_ROLL 0x20

void faller_init(unsigned char index);
void faller_update(unsigned char index);

#endif
