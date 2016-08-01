# maplegym

OpenAI's [Gym](https://gym.openai.com/docs) implements one common interface for
a wide range of environments  for reinforcement learning (e.g. old Atari games,
board games, algorithm challenges, etc.). You write the agent/algorithm and Gym
lets you you run it everywhere. maplegym adds
[MapleStory](http://maplestory.nexon.net) to Gym so you can teach programs
(these days, probably neural networks) to play for you.

## Installation

```
pip install gym maplegym
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
returns an action; Gym runs it; and a timestep later, gives you a 4-tuple
`(observation, reward, done, info)`.

 *  To slice a 60 FPS game into timesteps, we define a "step" as a 200ms pause
    after the action. You can change this by setting `maplegym.conf.timestep`
    (currently `0.2`).

 *  `observation` is an 600×800×3 uint8 array of screen pixel data.

 *  `action` must be in `range(20)`; see [this](actions.md).

 *  `reward` is how much your score—defined as your character's cumulative EXP
    plus his money as a uint32—has increased in the last timestep.

 *  `done` is True if the character has reached level 70.

 *  `info` is a dict usually containing diagnostic info, but maplegym doesn't
    use it.

## Implementation details

Coercing MapleStory into a black-box environment is a very noisy struggle, one
that requires relentless duct-taping and compromise.

maplegym uses an old version of the game (v.55) from about 2008. The
maplegym `Env` automatically starts a bundled MapleStory server that
handles gameplay basics and is **by no means a full-fledged server.**

In general, the idea is for the metagame to be removed completely, and for the
agent to focus on recognizing and fighting monsters. Thus...
    
 *  Your character is a Bandit.

 *  The game is restricted to a small set of maps; when the character reaches
    the right level, he's automatically teleported to the next map.

 *  Items, AP, and SP are disabled. Your stats and skills are raised
    automatically, your character automatically puts on new equipment,
    and when you pick up monster drops, you're
    immediately compensated for item's shop price.

 *  There's no HP or MP. When you're damaged or use a skill, you lose mesos
    equal to the amount of HP or MP lost. (You have a floor at 0.)

 *  Every single UI element is removed. The game screen is an image of nothing
    but you fighting monsters.

 *  All keys (and mouse clicks) are disabled except Control, Z, Alt, and the
    arrow keys, which correspond to Attack, Loot, Jump, and movement. Buffs
    are [re-]cast automatically, and your Control key is automatically remapped
    to your latest feasible attacking skill (Double Stab or Savage Blow). [1]

 *  The EXP curve is rewritten to be much more lenient, since the environment
    is supposed to be well-defined and return well-defined rewards, not take
    forever to beat.

This reduces the game to a bitmap state, a small set of abstract actions (e.g.
"attack"), and a unified reward structure.

Most of these tweaks are done on the server. Client-side modifications are made
using two DLLs, MapleController and MapleControllerLib. MapleController is
injected into MapleStory and installs a bunch of hooks to fake keypresses, skip
the login screen, remove UI elements, etc. MapleControllerLib is the Python/C
bridge and includes things like launching MapleStory and injecting
MapleController, sending keypresses, getting MapleStory's window bitmap data,
etc.

(A third uninteresting DLL, ForceWindowMode, does what its
name suggests. I considered using DXWnd, but it had a lot of unnecessary
features and didn't have an API.)

I also disabled the client's gratuitous packet encryption and removed 600 MB of
unused game data.

## Recorder

Many reinforcement learning algorithms start with a seed of past experiences so
the agent doesn't spend ages stumbling around like a human toddler. The
`maplegym.recorder.record` function lets you play the game yourself and record
your gameplay:

```python
import maplegym

for obs1, action, reward, obs2 in maplegym.recorder.record():
    # record the experience here
    pass
```

`maplegym.recorder.record` opens a MapleStory instance for you to play and
`yield`s a 4-tuple experience `(obs1, aciton, reward, obs2)` every timestep.
`obs1` is the 600×800×3 game screen before the timestep, `action` is the number
of the action you took, `reward` is your subsequent int32 reward, and `obs2` is
the game screen after. You can save them to a file or whatever.

## Roadmap

There's lots to do. A good chunk of maplegym is currently hardcoded in the
sense that it's not extensible. A full-fledged implementation, for example,
would probably include tools for autogenerating game files from a list of
maps/mobs/items, all jobs implemented, perhaps working quests, etc.

It all depends on how much attention this gets (I'm not hopeful), how motivated
I feel, and whether the capitalist grim reaper comes calling again soon. The
next hypothetical task is probably to turn everything into code (rather than
mysteriously patched binaries) so other people can contribute.

Pull requests and issues are warmly welcomed.

## Notes

[1] Actually, buffs are not currently autocast. There's about 20 lines of code
left to get it working. Someone open an issue to remind me.
