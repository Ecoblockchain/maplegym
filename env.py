import os
import time
import ctypes
import subprocess

import gym
import gym.spaces

import conf
from mapleinstance import MapleInstance

_actions = [
    'do_nothing', 'attack_still', 'loot_still', 'jump_still',
    'move_up',    'attack_up',    'loot_up',    'jump_up',
    'move_right', 'attack_right', 'loot_right', 'jump_right',
    'move_down',  'attack_down',  'loot_down',  'jump_down',
    'move_left',  'attack_left',  'loot_left',  'jump_left',
]

# _actions => range(20)
action_table = {x: i for i, x in enumerate(_actions)}


def colorspace(w, h, channels):
    return gym.spaces.Box(low=0, high=255, shape=(w, h, channels))


class MapleEnv(gym.Env):
    def __init__(self):
        self.observation_space = colorspace(w=800, h=525, channels=3)
        self.action_space = gym.spaces.Discrete(len(action_table))

        self.maple = MapleInstance()
        self.maple.start()

        if conf.show_screen:
            self.maple.show_screen()

        if conf.suspend_between_steps:
            self.maple.suspend()

        self._first_reset = True

    def _close(self):
        self.maple.stop()

    def _step(self, action):
        if conf.suspend_between_steps:
            self.maple.resume()

        self.maple.send_action(action)
        time.sleep(conf.timestep)
        reward, done = self.maple.get_reward()

        if conf.suspend_between_steps:
            self.maple.suspend()

        return self._get_obs(), reward, done, {}

    def _reset(self):
        self.score = 0
        self.maple.reset(self._first_reset)
        self._first_reset = False

        # wait for screen to render (ugly)
        time.sleep(0.5)  

        return self._get_obs()

    def _get_obs(self):
        return self.maple.get_screen()
