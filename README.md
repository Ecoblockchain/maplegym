# maplegym

OpenAI's [Gym](https://gym.openai.com/docs) implements a common, unified
interface for a wide range of environments (e.g. old Atari games, board games,
algorithm challenges, etc.) for reinforcement learning. You write the
agent/algorithm and Gym lets you you run it everywhere. maplegym adds
[MapleStory](http://maplestory.nexon.net) to Gym so you can teach programs
(these days, probably neural networks) to play for you.

## Installation

```
pip install maplegym
```

(On OS X and Linux, you may have to add `sudo`.)

You'll also have to download maplegym's grotesquely modified MapleStory client
from [here](#) and unzip it to `C:\Nexon\MapleStory`. Sorry, I know this sucks,
and I know the download is 200 MB; I'm fixing both. (I already got it down from
1 GB.)

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

Gym lets you implement the "agent-environment loop": it gives you an
`observation`, or the current "state" of the game; your agent reads it and
returns an action; Gym runs it, and gives you (1) the next `observation`, (2)
the resulting `reward`, (3) whether the game is `done`, and (4) some extra
info.

 *  To slice a continuous game (60 FPS) into discrete timesteps, we define a
    "step" as as 200 millisecond pause after the action. You can change this by setting
    `maplegym.conf.timestep`, which is currently `0.2`.

 *  `observation` **([[[int]]])** is an 600×800×3 array of screen pixel data.

 *  `action` **(int)** must be a number in `range(20)`, or:

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

 *  `reward` **(int)** is how much your score, which is defined as your
    character's cumulative EXP plus his money, has increased since the last
    timestep.

 *  `done` **(bool)** is True if the character has hit level 70.

 *  `info` **(dict)** lets environments send back diagnostic info, but maplegym
    doesn't use it.

## Implementation details

Coercing MapleStory into a black-box environment is a very noisy struggle. I
hacked and compromised relentlessly.

 *  maplegym uses an old version of the game (v.55) from about 2008. The
    maplegym `Env` automatically starts a bundled MapleStory server that
    handles gameplay basics and is **by no means a full-fledged server.**
    
 *  Your character is a Bandit.

 *  The game is restricted to a small set of maps. The character
    starts at level 1. When the character reaches the appropriate
    level, he is automatically teleported to the next map.

 *  Items, AP, and SP are disabled. Your stats and skills are raised
    automatically, and your character automatically puts on new equipment.

 *  There's no inventory. When you pick up monster drops, you're
    immediately compensated for item's shop price.

 *  There's no concept of HP or MP (your character stays at
    30000/30000). When you're damaged or use a skill, your money
    is deducted by the amount of HP or MP lost.

 *  All UI elements are removed from the game, including the
    status bar, all windows, the minimap, etc. The game screen is
    an image of nothing but you fighting monsters.

 *  All keys (and mouse clicks) are disabled except Control, Z, Alt,
    and the arrow keys, which correspond to Attack, Loot, Jump, and
    movement. Buffs are [re-]cast automatically, and your Control key
    is automatically remapped to your latest feasible attacking skill
    (Double Stab or Savage Blow). [1]

 *  The EXP curve is rewritten to be a couple orders of magnitude more lenient,
    since the goal is to create a well-defined environment that returns
    well-defined rewards, not to force the player to slave away.

This reduces the game to a bitmap of what's happening, a handful of commands,
and a unified reward structure.

I achieve these modifications using two DLLs, MapleController and
MapleControllerLib. MapleController is injected into MapleStory and installs a
bunch of hooks to fake keypresses, skip the login screen, remove UI elements,
etc. MapleControllerLib exports functions for Python to call and includes
things like launching MapleStory and injecting MapleController, sending
keypresses, getting MapleStory's window bitmap data, etc.

(There's also a third uninteresting DLL, ForceWindowMode, which does what its
name suggests. I considered using DXWnd, but it had a lot of unnecessary
features and didn't have an API.)

I also disabled the client's gratuitous packet encryption and removed 600 MB of
unused game data.

## Recorder

Many reinforcement learning algorithms prescribe seeding their experience pool
with a corpus of past experiences so the agent doesn't have to spend ages
stumbling around like a human toddler. The 
`maplegym.recorder.record` function lets you play the game yourself and
record your gameplay:

```python
import maplegym

for obs1, action, reward, obs2 in maplegym.recorder.record():
    # record the experience here
    pass
```

`obs1` is the 600×800×3 game screen before the timestep, `action` is the number
of the action you took, `reward` is your subsequent int32 reward, and `obs2` is
the game screen after. You can save them to a file or whatever.

## Closing thoughts and roadmap

MapleStory is an interesting environment for testing reinforcement learning
agents because of its colorful variety of maps and monsters and because your
character has different strengths throughout his evolution. These are all
important in developing general-purpose agents. At the same time, MapleStory is
still a 2D game with monsters and characters about ten frames large that are
easy to detect and very consistent textiles.

There's lots to do. A good chunk of maplegym is currently hardcoded in the
sense that it's not extensible. A full-fledged implementation, for example,
would probably include tools for autogenerating game files from a list of
maps/mobs/items, all jobs implemented, perhaps working quests, etc.

It all depends on how much attention this gets (I'm not hopeful), how motivated
I feel, and whether the capitalist grim reaper comes calling again soon. The
next hypothetical task is probably be to turn everything into code (rather than
mysteriously patched binaries) so other people can contribute.

Pull requests and issues are warmly welcomed.

## Notes

[1] Actually, buffs are not currently autocast. There's about 20 lines
of code needed to get it working. Someone open an issue to remind me.
