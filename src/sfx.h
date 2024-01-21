#ifndef SFX_H
#define SFX_H

#define ASD_STAGE_ATTACK  0
#define ASD_STAGE_SUSTAIN 1
#define ASD_STAGE_DECAY   2

#define SFXF_RANDOM_PITCH 0x01

#define SFX_MAX 24

// Predefined sound effects.
typedef struct sfx_t {

  // Pulse, sawtooth, triangle or noise.
  unsigned char waveform_type[SFX_MAX];

  // Waveform pulse width.
  // Lower values produce shorter pulses, which sound "thinner".
  // Pulse width is not relevant for the noise waveform.
  unsigned char pulse_width[SFX_MAX];

  // How long each pulse width sweep step takes, in frames. Negative times step down, positive times step up.
  unsigned char pulse_width_sweep_step_time[SFX_MAX];

  // 0 for negative sweep steps, 1 for positive steps.
  unsigned char pulse_width_sweep_direction[SFX_MAX];

  // VERA sound frequency.
  // sound frequency = 25000000 / (2^17) * frequency_word;
  unsigned int frequency[SFX_MAX];

  // Frequency change every frame.
  // new_frequency = previous_frequency +/- frequency_sweep_step * 2;
  unsigned char frequency_sweep_step[SFX_MAX];

  // 0 for negative frequency change, 1 for positive.
  unsigned char frequency_sweep_direction[SFX_MAX];

  // ADR sustain volume.
  unsigned char volume[SFX_MAX];

  // Volume envelope stages.
  unsigned char volume_attack_step[SFX_MAX];
  unsigned char volume_attack_step_len[SFX_MAX];

  unsigned char volume_sustain_len[SFX_MAX];

  unsigned char volume_decay_step[SFX_MAX];
  unsigned char volume_decay_step_len[SFX_MAX];

  // Flags.
  unsigned char flags[SFX_MAX];
} sfx_t;

extern sfx_t sfx;


#define SFX_CHANNEL_MAX 4

// PSG sound effect channels.
typedef struct sfx_channel_t {

  // Index of sfx_t that is playing.
  // 0xFF if nothing is playing.
  unsigned char sfx[SFX_CHANNEL_MAX];

  // Volume envelope gain.
  unsigned char gain[SFX_CHANNEL_MAX];

  // 0 - 63 VERA volume.
  unsigned char volume_left[SFX_CHANNEL_MAX];
  unsigned char volume_right[SFX_CHANNEL_MAX];

  // For channel allocation. Sounds played with a high priority will overwrite a lower priority channel if no channels are free to play it on.
  unsigned char priority[SFX_CHANNEL_MAX];

  // Volume ASD state.
  unsigned char asd_stage[SFX_CHANNEL_MAX];
  unsigned char asd_frames[SFX_CHANNEL_MAX];

  // VERA state to save on lookups.
  unsigned char pulse_width[SFX_CHANNEL_MAX];
  unsigned int frequency[SFX_CHANNEL_MAX];
} sfx_channel_t;

extern sfx_channel_t sfx_channels;


// Listener position.
extern unsigned char sfx_listen_x;
extern unsigned char sfx_listen_y;


void sfx_init();
void sfx_load(const char* filename);
void sfx_update();
void sfx_play_pan(const unsigned char sfx_index, const unsigned char priority, const unsigned char source_x, const unsigned char source_y);
void sfx_play(const unsigned char sfx_index, const unsigned char vol_left, const unsigned char vol_right, const unsigned char priority);
void sfx_vera_channel_update(const unsigned char chan);
void sfx_vera_channel_stop(const unsigned char chan);

#endif
