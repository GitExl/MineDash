import struct
from typing import List, Dict, BinaryIO, TextIO

from spritegroup import SpriteGroup


STATE_MAX = 256


class StateFlags:
    REPEAT_RANDOM = 0x01
    INVISIBLE = 0x02
    DESTROY = 0x04


class State:

    def __init__(self, index: int, sprite: int, duration: int, next_state_rel: int, flags: int):
        self.sprite: int = sprite
        self.duration: int = duration
        self.next_state: int = index + next_state_rel
        self.flags: int = flags

    @staticmethod
    def from_json(index: int, sprite_group: SpriteGroup, data):
        flags = 0

        if len(data[3]):
            flag_strs = data[3].split('|')
            for flag_str in flag_strs:
                if flag_str == 'REPEAT_RANDOM':
                    flags |= StateFlags.REPEAT_RANDOM
                elif flag_str == 'INVISIBLE':
                    flags |= StateFlags.INVISIBLE
                elif flag_str == 'DESTROY':
                    flags |= StateFlags.DESTROY
                else:
                    raise Exception('Unknown state flag {}'.format(flag_str))

        return State(index, sprite_group.index_for_name(data[0]), data[1], data[2], flags)


class StateList:

    def __init__(self, name: str):
        self._name: str = name
        self._labels: Dict[str, int] = {}
        self._states: List[State] = []

    def add_state(self, state: State):
        self._states.append(state)

        if len(self._states) > STATE_MAX:
            raise Exception('Too many states in {}.'.format(self._name))

    def add_label(self, label: str):
        self._labels[label] = len(self._states)

    def __len__(self):
        return len(self._states)

    @staticmethod
    def from_json(name: str, sprite_group: SpriteGroup, data):
        states = StateList(name)

        for label, state_data_list in data.items():
            states.add_label(label)

            for state_data in state_data_list:
                states.add_state(State.from_json(
                    len(states),
                    sprite_group,
                    state_data
                ))

        return states

    def write_data(self, f: BinaryIO):
        s = struct.Struct('B')

        # Sprite indices.
        for state in self._states:
            f.write(s.pack(state.sprite))
        for pad in range(0, STATE_MAX - len(self._states)):
            f.write(b'\x00')

        # Duration.
        for state in self._states:
            f.write(s.pack(state.duration))
        for pad in range(0, STATE_MAX - len(self._states)):
            f.write(b'\x00')

        # Next state.
        for state in self._states:
            f.write(s.pack(state.next_state))
        for pad in range(0, STATE_MAX - len(self._states)):
            f.write(b'\x00')

        # Flags
        for state in self._states:
            f.write(s.pack(state.flags))
        for pad in range(0, STATE_MAX - len(self._states)):
            f.write(b'\x00')

    def write_labels(self, f: TextIO):
        for label, index in self._labels.items():
            f.write('#define ST_{}_{} {}\n'.format(self._name, label, index))
