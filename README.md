# maplegym

OpenAI's Gym lets you control a wide array of environments for reinforcement
learning (like old Atari games, board games, algorithm challenges, etc.)
through a common interface. You simply write the agent/algorithm, and Gym lets
you test it on all the different environments in ten lines of code.
For more, see their [documentation](https://gym.openai.com/docs).

maplegym adds [MapleStory](http://maplestory.nexon.net) as an
environment to Gym. This lets you teach programs
(these days, probably neural networks) to play MapleStory for you.

## Installation

```
pip install maplegym
```

(On OS X and Linux, you may have to add `sudo`.)

## Usage

Here's the skeleton of your program:

```python
import gym
import maplegym  # importing this automatically registers it with gym

env = gym.make('MapleGym-v0')

observation = env.reset()
done = False

while not done:
    action = get_next_action(observation)  # for you to implement
    observation, reward, done, info = env.step(action)
```

As seen, Gym gives you an `observation`, which is the current "state"
of the game. Using this your agent comes up with an `action`. Gym runs
this action and gives you the next `observation`, the resulting `reward`,
whether the game is `done`, and extra info.

 *  `observation`s are simply the 800x600x3 pixel data of the game (3 RGB
    channels). 

 *  `action` must be a number `0 <= n < 20`. These are:

    | Number | Action |
    | ------ | ------ |
    | 0      | `do_nothing` |
    | 1      | `attack_still` |
    | 2      | `loot_still` |
    | 3      | `jump_still` |
    | 4      | `move_up` |
    | 5      | `attack_up` |
    | 6      | `loot_up` |
    | 7      | `jump_up` |
    | 8      | `move_right` |
    | 9      | `attack_right` |
    | 10     | `loot_right` |
    | 11     | `jump_right` |
    | 12     | `move_down` |
    | 13     | `attack_down` |
    | 14     | `loot_down` |
    | 15     | `jump_down` |
    | 16     | `move_left` |
    | 17     | `attack_left` |
    | 18     | `loot_left` |
    | 19     | `jump_left` |

 *  `reward` is a number. The game keeps track of your score, which is defined
    as your character's cumulative experience points plus the number of mesos (money) it has.

 *  `done` is True if the character has hit level 70.

 *  `info` will normally hold extra info, but maplegym doesn't currently use it (it'll be an empty dict).

## Implementation details

Coercing MapleStory into a black-box environment is rough. I made a
good number of tweaks and compromises:

 *  I'm using an old version of the game (55) from about 2008. The maplegym `Env`
    automatically starts a bundled MapleStory server that handles
    gameplay basics. **It is by no means a full-fledged server.** Your
    character is a Bandit.

 *  The game is restricted to a small set of maps. The character
    starts at level 1. When the character reaches the appropriate
    level, he is automatically teleported to the next map.

 *  Items, AP, and SP are disabled. Your stats and skills are raised
    automatically, and when your character reaches the right level, he
    immediately puts on new equipment.

 *  There is no inventory. When you pick up monster drops, you're
    immediately compensated for item's shop price.

 *  There is no concept of HP or MP (your character stays at
    30000/30000). When you are damaged or use a skill, your meso count
    is deducted by the amount of HP or MP lost.

 *  All UI elements are removed from the game, including the
    status bar, all windows, the minimap, etc. The game screen is
    an image of nothing but you fighting monsters.

 *  All keys (and mouse clicks) are disabled except Control, Z, Alt,
    and the arrow keys, which correspond to Attack, Loot, Jump, and
    movement. Buffs are cast automatically, and your Control key
    is automatically remapped to your latest feasible attacking skill
    (Double Stab or Savage Blow). [1]

 *  The EXP curve is completely written to be a couple orders of
    magnitude more lenient, since the goal is to create
    a well-defined environment that returns well-defined rewards, not
    to force the player to slave away.

Importantly, the `observation` is simply a full bitmap of what's
happening in the environment, the `reward` is defined solely in
terms of EXP and mesos, and the `action` is an abstract command in an
abstract direction.

I achieve these modifications using two DLLs, MapleController and
MapleControllerLib. MapleController is injected into MapleStory and
installs a bunch of hooks to fake keypresses, skip the login screen,
remove UI elements, etc. MapleControllerLib exports functions for
Python to call and includes things like launching MapleStory and
injecting MapleController, sending keypresses, getting MapleStory's
window bitmap data, etc.

(There's also a third uninteresting DLL, ForceWindowMode, which does
what its name suggests. I considered using DXWnd, but it had a lot
of unnecessary features and didn't have an API.)

I also heavily modified the client. I disabled its packet encryption,
because all traffic goes through localhost, and removed 600 MB of
unused game data.

## Recorder

Many reinforcement learning algorithms seed their neural network with
a memory of past experiences so the agent can start out better than
just stumbling around like a toddler. maplegym includes the
`maplegym.recorder.record` function for you to play the game yourself
and record your gameplay:

```python
import maplegym

for obs1, action, reward, obs2 in maplegym.recorder.record():
    # record the experience here
    pass
```

`obs1` is the game screen (the 800x600x3 array) before the
timestep, `action` is the number of the action you took, `reward`
is your subsequent reward as a 32-bit integer, and `obs2` is the game
screen after. You can do anything with them, like save them to a file
or database.

## Closing thoughts & todo list

MapleStory is an interesting environment for testing reinforcement
learning agents because it has a wide variety of maps with monsters of
different sizes/colors/abilities and your character has different
strengths throughout his evolution.  These are all important in
developing agents capable of generalizing their behavior. At the same
time, MapleStory is still a 2D game, with monsters and characters
about ten frames large that are very easy to detect, and very
consistent textiles.

There's lots to be done. A good portion of maplegym is currently
hardcoded in the sense that it is not extensible. A full-fledged
implementation, for example, would probably have tools for
autogenerating game files using a list of maps/mobs/items,
all the jobs implemented, perhaps working quests, etc.

It all depends on how much attention this gets (I am not hopeful) and
how motivated I feel. If this does grow, the next thing to do will
probably be to turn everything into code (rather than mysteriously
patched binaries) so other people can contribute.

## Notes

[1] Actually, buffs are not currently autocast. There's about 20 lines
of code needed to get it working. Someone open an issue to remind me.
