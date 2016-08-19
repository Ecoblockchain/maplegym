import gym
from PIL import Image
import numpy as np
import maplegym
import time

maplegym.conf.show_screen = True
maplegym.conf.suspend_between_steps = False

env = gym.make('MapleGym-v0')
obs = env.reset()

arr = np.reshape(np.array(obs, dtype=np.uint8), (600, 800, 3))
img = Image.fromarray(arr)
img.save('screenshot.jpg')
