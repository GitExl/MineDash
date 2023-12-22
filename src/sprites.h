#ifndef SPRITES_H
#define SPRITES_H

#define SPRITE_MAX 128

typedef struct sprite_t {
  unsigned int address[SPRITE_MAX];
  unsigned char size_palette[SPRITE_MAX];
} sprite_t;

extern sprite_t sprites;

#endif
