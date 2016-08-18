import random
import time
import sys
import gym
import maplegym

actions_rev = {y: x for x, y in maplegym.action_table.iteritems()}

maplegym.conf.show_screen = True
maplegym.conf.suspend_between_steps = False


class Agent:
    def __init__(self):
        self.gen = self.get_actions()

    def get_actions(self):
        while True:
            a = random.choice(maplegym.action_table.values())
            print 'doing action for 50 rounds: %s' % actions_rev[a]
            for _ in xrange(50):
                yield a

    def act(self, obs):
        return self.gen.next()


def run_episode(env, agent):
    obs = env.reset()
    while True:
        action = agent.act(obs)
        obs, reward, done, info = env.step(action)
        if done:
            return


if __name__ == '__main__':
    env = gym.make('MapleGym-v0')
    agent = Agent()

    for episode in range(100):
        run_episode(env, agent)
