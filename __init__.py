from env import action_table
from gym.envs.registration import registry, register, make, spec

import conf
import recorder

register(
    id='MapleGym-v0',
    entry_point='maplegym.env:MapleEnv',
    nondeterministic=True
)
