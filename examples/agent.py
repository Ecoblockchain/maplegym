import random
import sys
import gym
import maplegym

actions = maplegym.possible_actions
r_actions = {y: x for x, y in actions.iteritems()}

maplegym.conf.show_screen = True


class Agent:
    def __init__(self):
        self.gen = self.get_actions()

    def get_actions(self):
        while True:
            a = random.choice(maplegym.possible_actions.values())
            print 'doing action for 50 rounds: %s' % r_actions[a]
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
