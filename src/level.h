#ifndef LEVEL_H
#define LEVEL_H

#define TILE_MAX 64

#define TILEF_BLOCKS       0x01
#define TILEF_SOFT         0x02
#define TILEF_LETHAL       0x04
#define TILEF_SPECIAL      0x08
#define TILEF_DESTRUCTIBLE 0x10
#define TILEF_GRAVITY      0x20
#define TILEF_PUSHABLE     0x40

#define GF_ABOVE 0x01
#define GF_LEFT  0x02
#define GF_RIGHT 0x04

#define TILE_INDEX(x, y) ((y & 0xFF) << 6) + (x & 0xFF)

typedef struct level_info_t {
  unsigned int left;
  unsigned int top;
  unsigned int right;
  unsigned int bottom;
  int points;
  unsigned char time_minutes;
  unsigned char time_seconds;
} level_info_t;

extern level_info_t level_info;

typedef struct tileset_t {
  unsigned char palette[TILE_MAX];
  unsigned char flags[TILE_MAX];
} tileset_t;

extern tileset_t tileset;

extern unsigned char level_tile[4096];
extern unsigned char level_owner[4096];

extern unsigned char entity_player;

extern unsigned char level_tile_x;
extern unsigned char level_tile_y;

extern unsigned char level_current;
extern unsigned char level_next;

void level_load(const unsigned char level);
void level_load_graphics();
void level_tile_clear(const unsigned char tile_x, const unsigned char tile_y);
void level_tile_special(const unsigned char tile_x, const unsigned char tile_y);
void level_tile_set(const unsigned char tile_x, const unsigned char tile_y, const unsigned char tile);
unsigned char level_tile_is_blocked(const unsigned char tile_x, const unsigned char tile_y);
unsigned char level_tile_flags(const unsigned char tile_x, const unsigned char tile_y);
unsigned char level_gravity_evaluate(const unsigned char tile_x, const unsigned char tile_y, const unsigned char gravity_flags);
void level_soft_evaluate(const unsigned char tile_x, const unsigned char tile_y);
void level_hud_build();
void level_update();
unsigned char level_tile_can_roll(unsigned int tile_index, const signed char side);

#endif
