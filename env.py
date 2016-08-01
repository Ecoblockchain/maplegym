import os
import time
import subprocess
import ctypes

import gym
import gym.spaces
import numpy as np

import conf
from mapleinstance import MapleInstance

possible_actions = [
    'do_nothing', 'attack_still', 'loot_still', 'jump_still',
    'move_up',    'attack_up',    'loot_up',    'jump_up',
    'move_right', 'attack_right', 'loot_right', 'jump_right',
    'move_down',  'attack_down',  'loot_down',  'jump_down',
    'move_left',  'attack_left',  'loot_left',  'jump_left',
]

possible_actions = {x: i for i, x in enumerate(possible_actions)}


def colorspace(w, h, channels):
    return gym.spaces.Box(low=0, high=255, shape=(w, h, channels))


class MapleEnv(gym.Env):
    def __init__(self):
        self.maple = MapleInstance()
        self.maple.start()
        if conf.show_screen:
            self.maple.show_screen()

        self.observation_space = colorspace(800, 525, 3)
        self.action_space = gym.spaces.Discrete(len(possible_actions))

        self._first_reset = True

    def _close(self):
        self.maple.stop()

    def _step(self, action):
        self.maple.send_action(action)
        time.sleep(conf.timestep)

        reward, done = self.maple.get_reward()
        return self._get_obs(), reward, done, {}

    def _reset(self):
        self.score = 0
        self.maple.reset(self._first_reset)
        self._first_reset = False
        return self._get_obs()

    def _get_obs(self):
        screen = self.maple.get_screen()
        return np.reshape(screen[:800 * 525 * 3], (525, 800, 3))
