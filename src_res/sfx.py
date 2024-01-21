import struct
from enum import Enum
from math import ceil, log
from typing import Dict, BinaryIO, TextIO


SFX_MAX = 24

FREQ_CONV = (48828.125 / (2 ** 17))


class SfxWaveform(Enum):
    PULSE = 0
    SAWTOOTH = 1
    TRIANGLE = 2
    NOISE = 3


class SfxFlags:
    RANDOM_PITCH = 0x01


class Sfx:
    def __init__(self, name: str, data):
        self.name: str = name
        self.flags: int = 0

        self.waveform: SfxWaveform = SfxWaveform(data['waveform_type'])
        self.pulse_width: int = data['pulse_width']

        # pulse_width_sweep_step_time subtracted by one so that we can AND it for a quick modulo check.
        pulse_width_sweep_step_time = abs(data['pulse_width_sweep_time'])
        if pulse_width_sweep_step_time != 0:
            pulse_width_sweep_step_time -= 1
        self.pulse_width_sweep_step_time: int = pulse_width_sweep_step_time
        self.pulse_width_sweep_direction: int = 0 if data['pulse_width_sweep_time'] < 0 else 1

        self.frequency: int = ceil(data['frequency'] / FREQ_CONV)
        self.frequency_sweep_step: int = ceil(abs(data['frequency_sweep_step']) / FREQ_CONV)
        self.frequency_sweep_direction: int = 0 if data['frequency_sweep_step'] < 0 else 1

        if data.get('random_pitch', True):
            self.flags |= SfxFlags.RANDOM_PITCH

        self.volume: int = min(63, ceil(data['volume'] * 63))

        attack_len = data['volume_attack_len']
        if attack_len > self.volume:
            attack_step = 1
            attack_step_len = attack_len / self.volume
        elif attack_len:
            attack_step = self.volume / attack_len
            attack_step_len = 1
        else:
            attack_step = self.volume
            attack_step_len = 0
        self.volume_attack_step: int = ceil(attack_step)
        self.volume_attack_step_len: int = ceil(attack_step_len)

        self.volume_sustain_len: int = data['volume_sustain_len']

        decay_len = data['volume_decay_len']
        if decay_len > self.volume:
            decay_step = 1
            decay_step_len = decay_len / self.volume
        elif decay_len:
            decay_step = self.volume / decay_len
            decay_step_len = 1
        else:
            decay_step = 0
            decay_step_len = 0
        self.volume_decay_step: int = ceil(decay_step)
        self.volume_decay_step_len: int = ceil(decay_step_len)

        total_len = (ceil(self.volume / self.volume_attack_step) * self.volume_attack_step_len) + self.volume_sustain_len + (ceil(self.volume / self.volume_decay_step) * self.volume_decay_step_len) + 3

        sweep_total = self.frequency_sweep_step * total_len
        if not self.frequency_sweep_direction:
            sweep_total = -sweep_total
        if self.frequency + sweep_total < 0 or self.frequency + sweep_total > 65535:
            print('** Warning: SFX {} frequency sweep will probably wrap.'.format(self.name))

        if pulse_width_sweep_step_time:
            pulse_total = ceil(total_len / self.pulse_width_sweep_step_time)
            if not self.pulse_width_sweep_direction:
                pulse_total = -pulse_total
            if self.pulse_width + pulse_total < 0 or self.pulse_width + pulse_total > 63:
                print('** Warning: SFX {} pulse sweep will probably wrap.'.format(self.name))


class SfxList:

    def __init__(self, name: str, data):
        self._name: str = name
        self._sfx: Dict[str, Sfx] = {}

        for name, sfx_data in data.items():
            self._sfx[name] = Sfx(name, sfx_data)

    def write_labels(self, f: TextIO):
        for index, label in enumerate(self._sfx.keys()):
            f.write('#define SFX_{}_{} {}\n'.format(self._name, label, index))

    def write_data(self, f: BinaryIO):
        sb = struct.Struct('B')
        sh = struct.Struct('H')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.waveform.value))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.pulse_width))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.pulse_width_sweep_step_time))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.pulse_width_sweep_direction))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sh.pack(sfx.frequency))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.frequency_sweep_step))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.frequency_sweep_direction))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.volume))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.volume_attack_step))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.volume_attack_step_len))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.volume_sustain_len))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.volume_decay_step))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.volume_decay_step_len))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')

        for sfx in self._sfx.values():
            f.write(sb.pack(sfx.flags))
        for pad in range(0, SFX_MAX - len(self._sfx)):
            f.write(b'\x00')
