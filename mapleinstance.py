import os
import time
import ctypes

import conf
import mapleserver


def create_controller():
    path = 'MapleController/Release/MapleControllerLib.dll'
    return ctypes.CDLL(os.path.join(os.path.dirname(__file__), path))


class MapleInstance:
    def __init__(self):
        self.controller = create_controller()
        self._score = 0

    def start(self):
        print '[maplegym] Starting MapleStory...'
        self.server = mapleserver.start()

        exe = unicode(os.path.join(conf.maple_folder, 'localhost.exe'))
        self.controller.RunMapleStoryW(exe)
        self.server.wait_login()

        print '[maplegym] MapleStory is now running.'

    def __del__(self):
        self.stop()

    def stop(self):
        self.controller.TerminateMapleStory()
        self.server.stop()
        print '[maplegym] MapleStory was terminated.'

    def reset(self, first):
        self._score = 0
        print 'resetting char'
        mapleserver.levels.reset_character(self.player, warp=not first)

    def show_screen(self):
        self.controller.ShowMapleWindow()

    def _suspend(self, suspend):
        self.controller.SuspendMapleStory(ctypes.c_int(suspend))

    def suspend(self):
        self._suspend(True)

    def resume(self):
        self._suspend(False)

    @property
    def player(self):
        return self.server.state.player

    # ...

    def get_screen(self):
        buf = ctypes.create_string_buffer(800 * 600 * 3)
        self.controller.GetMapleScreen(buf)
        return bytearray(buf.raw)

    def send_action(self, a):
        self.controller.DoAction(ctypes.c_ushort(a))

    def get_last_action(self):
        return int(self.controller.GetLastAction())

    def cumulative_exp(self, player):
        total = sum(mapleserver.conf.exp_table[:player.level - 1])
        return total + player.exp

    def _get_score(self):
        player = self.player.s.char

        score = self.cumulative_exp(player) + player.inventory.mesos
        done = (player.level >= 70)
        return score, done

    def get_reward(self):
        score, done = self._get_score()
        reward = score - self._score
        self._score = score
        return reward, done
