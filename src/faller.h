#ifndef FALLER_H
#define FALLER_H

#define FALLER_TYPE_ROCK    0b00000000
#define FALLER_TYPE_GOLD    0b00000100
#define FALLER_TYPE_DIAMOND 0b00001000

#define FALLER_STATE_IDLE       0b00000000
#define FALLER_STATE_FALL       0b00100000
#define FALLER_STATE_ROLL_LEFT  0b01000000
#define FALLER_STATE_ROLL_RIGHT 0b01100000
#define FALLER_STATE_PUSH_LEFT  0b10000000
#define FALLER_STATE_PUSH_RIGHT 0b10100000

#define FALLER_DATA_DELAY 0b00000011
#define FALLER_DATA_TYPE  0b00011100
#define FALLER_DATA_STATE 0b11100000

void faller_init(const unsigned char index);
void faller_update(const unsigned char index);
unsigned char faller_type_for_tile(const unsigned char tile);
void faller_set_state(const unsigned char index, const unsigned char local_state, const unsigned char local_type);

#endif
