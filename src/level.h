#ifndef LEVEL_H
#define LEVEL_H


#define TILE_INDEX(x, y) ((y & 0xFF) << 6) + (x & 0xFF)

#define MAP_WIDTH 64
#define MAP_HEIGHT 64


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


#define TILE_MAX 64

#define TILEF_BLOCKS       0x01
#define TILEF_SOFT         0x02
#define TILEF_LETHAL       0x04
#define TILEF_SPECIAL      0x08
#define TILEF_DESTRUCTIBLE 0x10
#define TILEF_GRAVITY      0x20
#define TILEF_PUSHABLE     0x40

typedef struct tileset_t {
  unsigned char palette[TILE_MAX];
  unsigned char flags[TILE_MAX];
} tileset_t;

extern tileset_t tileset;


typedef struct map_t {
  unsigned char tile[MAP_WIDTH * MAP_HEIGHT];
  unsigned char owner[MAP_WIDTH * MAP_HEIGHT];
} map_t;

extern map_t map;


extern unsigned char entity_player;

extern unsigned char level_current;
extern unsigned char level_next;

extern unsigned char level_hud_update;


#define GF_ABOVE 0x01
#define GF_LEFT  0x02
#define GF_RIGHT 0x04
#define GF_CRUSH 0x08


void level_load(const unsigned char level);
void level_load_graphics();
void level_tile_clear(const unsigned char tile_x, const unsigned char tile_y);
void level_tile_execute_special(const unsigned char tile_x, const unsigned char tile_y);
void level_tile_set(const unsigned int tile_index, const unsigned char tile);
unsigned char level_tile_is_blocked(const unsigned char tile_x, const unsigned char tile_y);
unsigned char level_tile_flags(const unsigned char tile_x, const unsigned char tile_y);
void level_tile_evaluate_faller(const unsigned char tile_x, const unsigned char tile_y, const unsigned char gravity_flags);
unsigned char level_tile_evaluate_gravity(unsigned int tile_index, const unsigned char gravity_flags);
void level_tile_evaluate_dirt(const unsigned char tile_x, const unsigned char tile_y);
void level_hud_build();
void level_update();
unsigned char level_tile_can_roll(unsigned int tile_index, const signed char side);
unsigned char level_tile_push(const unsigned char tile_x, const unsigned char tile_y, const signed char direction);
void level_tile_touch(const unsigned char tile_x, const unsigned char tile_y, const unsigned char entity_type_flags, const signed char move_x, const signed char move_y);
void level_tile_explode(unsigned char x, unsigned char y);
void level_tile_start_explosion(unsigned char x, unsigned char y);

#endif
