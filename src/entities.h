#ifndef ENTITIES_H
#define ENTITIES_H

// Maximum number of entities.
#define ENTITY_MAX 64

// Entity flags.
#define ENTITYF_UNUSED     0x01   // This entity is unused.
#define ENTITYF_INVISIBLE  0x02   // This entity is not visible.
#define ENTITYF_FLIPX      0x04   // This entity is flipped on the X axis.
#define ENTITYF_FLIPY      0x08   // This entity is flipped on the Y axis.
#define ENTITYF_BEHIND_MAP 0x10   // This entity is rendered behind the map tile layer.

typedef struct entity_t {
  unsigned char type[ENTITY_MAX];
  unsigned char flags[ENTITY_MAX];
  unsigned char data[ENTITY_MAX];

  // State tracking.
  unsigned char state[ENTITY_MAX];
  unsigned char counter[ENTITY_MAX];

  // Tile position.
  signed char p_x[ENTITY_MAX];
  signed char p_y[ENTITY_MAX];
  unsigned char tile_x[ENTITY_MAX];
  unsigned char tile_y[ENTITY_MAX];
} entity_t;

extern entity_t entities;


#define ENTITY_TYPE_MAX 16

#define ETF_OWNERSHIP 0x01
#define ETF_CRUSHABLE 0x02
#define ETF_SPECIAL   0x04
#define ETF_DIGS      0x08
#define ETF_SLOW      0x10

typedef struct entity_type_t {
  unsigned char flags[ENTITY_TYPE_MAX];
} entity_type_t;

extern entity_type_t entity_types;


// Maximum number of states.
#define STATE_MAX 256

// State flags.
#define STATEF_RANDOM_REPEAT 0x01   // 50% chance to repeat this state.
#define STATEF_INVISIBLE     0x02   // This state is invisible.
#define STATEF_DESTROY       0x04   // The entity will be destroyed after this state.
#define STATEF_EXPLODE_TILE  0x08   // The entity will explode the tile it is on.

typedef struct entity_states_t {

  // Sprite index to display in this state.
  unsigned char sprite[STATE_MAX];

  // State duration in frames. 0 = infinite.
  unsigned char duration[STATE_MAX];

  // Next state.
  unsigned char next[STATE_MAX];

  // Flags for this state.
  unsigned char flags[STATE_MAX];
} entity_states_t;

extern entity_states_t entity_states;


#define TILE_MOVE_NO_EVALUATE 0x01   // Do not evaluate cleared tile after moving away from it.


void entities_update_vera_sam();
void entities_update();
void entities_load(const char* entity_filename);
void entities_load_states(const char* states_filename);
void entities_set_state(const unsigned char entity, const unsigned char state);
void entities_free(const unsigned char index);
unsigned char entities_spawn(const unsigned char type, const unsigned char tile_x, const unsigned char tile_y, const unsigned char flags, const unsigned char data);
void entities_init_entity(const char index, const char type);
void entities_set_invisible(const unsigned char entity);
void entities_set_behind_map(const unsigned char entity);
void entity_get_property_mask(const unsigned char entity, const unsigned char state_index);
void entities_tile_move(const unsigned char entity, const signed char move_x, const signed char move_y, const unsigned char move_flags);
void entities_crush(const unsigned char entity);
void entities_explode(const unsigned char entity);

#endif
