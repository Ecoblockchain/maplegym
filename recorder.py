import time
import conf
from mapleinstance import MapleInstance


def step(maple):
    s = maple.get_screen()
    time.sleep(conf.timestep)
    a = maple.get_last_action()
    r, done = maple.get_reward()
    s2 = maple.get_screen()

    return s, a, r, s2, done


def record():
    maple = MapleInstance()
    maple.start()
    maple.show_screen()

    while True:
        try:
            s, a, r, s2, done = step(maple)
            yield s, a, r, s2
            if done:
                maple.reset(False)
        except KeyboardInterrupt:
            break
