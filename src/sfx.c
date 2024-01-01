#include <cbm.h>
#include <cx16.h>

#include "sfx.h"
#include "sfx_pan.h"

sfx_t sfx;

sfx_channel_t sfx_channels;

unsigned char sfx_frame = 0;

unsigned char sfx_listen_x = 0;
unsigned char sfx_listen_y = 0;

static unsigned char i;

void sfx_init() {
  for (i = 0; i < SFX_CHANNEL_MAX; i++) {
    sfx_channels.sfx[i] = 0xFF;
  }
}

void sfx_load(const char* filename) {
  cbm_k_setnam(filename);
  cbm_k_setlfs(0, 8, 2);
  cbm_k_load(0, (unsigned int)&sfx);
}

void sfx_update() {
  unsigned char sfx_index;
  unsigned char asd_stage;
  unsigned char asd_frames;
  unsigned char gain;
  unsigned char pulse_width;
  unsigned char volume;
  unsigned int frequency;

  for (i = 0; i < SFX_CHANNEL_MAX; i++) {
    if (sfx_channels.sfx[i] == 0xFF) {
      continue;
    }

    sfx_index = sfx_channels.sfx[i];
    asd_stage = sfx_channels.asd_stage[i];
    asd_frames = sfx_channels.asd_frames[i];
    gain = sfx_channels.gain[i];
    frequency = sfx_channels.frequency[i];
    pulse_width = sfx_channels.pulse_width[i];

    volume = sfx.volume[sfx_index];

    if (!asd_frames) {
      if (asd_stage == ASD_STAGE_ATTACK) {
        gain += sfx.volume_attack_step[sfx_index];
        if (gain >= volume) {
          gain = volume;
          asd_stage = ASD_STAGE_SUSTAIN;
          asd_frames = sfx.volume_sustain_len[sfx_index];
        } else {
          asd_frames = sfx.volume_attack_step_len[sfx_index];
        }
      } else if (asd_stage == ASD_STAGE_SUSTAIN) {
        asd_stage = ASD_STAGE_DECAY;
      } else if (asd_stage == ASD_STAGE_DECAY) {
        gain -= sfx.volume_decay_step[sfx_index];
        if (!gain || gain > 0x3F) {
          sfx_vera_channel_stop(i);
          continue;
        } else {
          asd_frames = sfx.volume_decay_step_len[sfx_index];
        }
      }
    } else {
      --asd_frames;
    }

    // Pulse width sweep.
    if (sfx_frame & sfx.pulse_width_sweep_step_time[sfx_index]) {
      if (sfx.frequency_sweep_direction[sfx_index]) {
        ++pulse_width;
      } else {
        --pulse_width;
      }
    }

    // Frequency sweep.
    // TODO: frequency step is in Hz, frequency value is not
    if (sfx.frequency_sweep_direction[sfx_index]) {
      frequency += sfx.frequency_sweep_step[sfx_index];
    } else {
      frequency -= sfx.frequency_sweep_step[sfx_index];
    }

    sfx_channels.asd_stage[i] = asd_stage;
    sfx_channels.asd_frames[i] = asd_frames;
    sfx_channels.gain[i] = gain;
    sfx_channels.frequency[i] = frequency;
    sfx_channels.pulse_width[i] = pulse_width;

    sfx_vera_channel_update(i);
  }

  ++sfx_frame;
}

void sfx_play_pan(const unsigned char sfx_index, const unsigned char priority, const unsigned char source_x, const unsigned char source_y) {
  const unsigned char distx = (sfx_listen_x - source_x) + 64;
  unsigned char vol_left = pan_l[distx];
  unsigned char vol_right = pan_r[distx];

  // Attenuate based on vertical distance.
  signed char disty = (sfx_listen_y - source_y);
  if (disty < 0) {
    disty = -disty;
  }
  disty = 63 - (disty & 0x3F);
  vol_left = (disty * vol_left) >> 6;
  vol_right = (disty * vol_right) >> 6;

  sfx_play(sfx_index, vol_left, vol_right, priority);
}

void sfx_play(const unsigned char sfx_index, const unsigned char vol_left, const unsigned char vol_right, const unsigned char priority) {
  unsigned char use = 0xFF;

  // Use the first unused channel or take over a channel playing a lower priority sound.
  for (i = 0; i < SFX_CHANNEL_MAX; i++) {
    if (sfx_channels.sfx[i] == 0xFF) {
      use = i;
      break;
    } else if (priority > sfx_channels.priority[i]) {
      use = i;
    }
  }
  if (use == 0xFF) {
    return;
  }

  sfx_channels.sfx[use] = sfx_index;
  sfx_channels.priority[use] = priority;
  sfx_channels.asd_stage[use] = ASD_STAGE_ATTACK;
  sfx_channels.asd_frames[use] = sfx.volume_attack_step_len[sfx_index];
  sfx_channels.gain[use] = 0;
  sfx_channels.volume_left[use] = vol_left & 0x3F;
  sfx_channels.volume_right[use] = vol_right & 0x3F;
  sfx_channels.frequency[use] = sfx.frequency[sfx_index];
  sfx_channels.pulse_width[use] = sfx.pulse_width[sfx_index] & 0x3F;

  sfx_vera_channel_update(use);
}

void sfx_vera_channel_update(const unsigned char chan) {
  static unsigned char wave_pulse;
  static unsigned char volume_left;
  static unsigned char volume_right;
  static unsigned char fq_lo, fq_hi;

  const unsigned char sfx_index = sfx_channels.sfx[chan];

  fq_lo = sfx_channels.frequency[chan] & 0xFF;
  fq_hi = (sfx_channels.frequency[chan] >> 8) & 0xFF;
  wave_pulse = (sfx.waveform_type[sfx_index] << 6) | sfx_channels.pulse_width[chan];

  // Calculate pan volumes for each side channel.
  volume_left = ((sfx_channels.gain[chan] * sfx_channels.volume_left[chan]) >> 6) & 0x3F;
  volume_right = ((sfx_channels.gain[chan] * sfx_channels.volume_right[chan]) >> 6) & 0x3F;

  // Write for both channel, left and right adjacent.
  VERA.address = 0xF9C0 + (chan * 8);
  VERA.address_hi = 0x01 | VERA_INC_1;

  VERA.data0 = fq_lo;
  VERA.data0 = fq_hi;
  VERA.data0 = 0b01000000 | volume_left;
  VERA.data0 = wave_pulse;

  VERA.data0 = fq_lo;
  VERA.data0 = fq_hi;
  VERA.data0 = 0b10000000 | volume_right;
  VERA.data0 = wave_pulse;
}

void sfx_vera_channel_stop(const unsigned char chan) {
  sfx_channels.sfx[chan] = 0xFF;

  VERA.address = 0xF9C2 + (chan * 8);
  VERA.address_hi = 0x01 | VERA_INC_4;

  VERA.data0 = 0;
  VERA.data0 = 0;
}
