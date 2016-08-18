import numpy as np
import struct
import maplegym

actions = {y: x for x, y in maplegym.action_table.iteritems()}


def write(f, s, a, r, s2):
    f.write(bytearray(np.reshape(s, 800 * 600 * 3)))
    f.write(struct.pack('<B', a))
    f.write(struct.pack('<l', r))
    f.write(bytearray(np.reshape(s2, 800 * 600 * 3)))


if __name__ == '__main__':
    with open('experiences.dat', 'wb') as f:
        for s, a, r, s2 in maplegym.recorder.record():
            write(f, s, a, r, s2)
            if a > 0 or r > 0:
                print '%s, reward %d' % (actions[a], r)
