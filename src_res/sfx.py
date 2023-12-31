import struct
from enum import Enum
from math import ceil, log
from typing import Dict, BinaryIO, TextIO


SFX_MAX = 16

FREQ_CONV = (48828.125 / (2 ** 17))


class SfxWaveform(Enum):
    PULSE = 0
    SAWTOOTH = 1
    TRIANGLE = 2
    NOISE = 3


class Sfx:
    def __init__(self, data):
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


class SfxList:

    def __init__(self, name: str, data):
        self._name: str = name
        self._sfx: Dict[str, Sfx] = {}

        for name, sfx_data in data.items():
            self._sfx[name] = Sfx(sfx_data)

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
