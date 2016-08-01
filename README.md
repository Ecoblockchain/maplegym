# maplegym

maplegym adds [MapleStory](http://maplestory.nexon.net) to [OpenAI
Gym](https://gym.openai.com/) so you can teach programs (these days,
probably neural networks) to play for you.

## Installation

```
pip install gym
git clone https://github.com/brhs/maplegym
```

Move the maplegym folder into your project directory so you can
`import` it.

Then, download maplegym's [modified MapleStory client](#) and unzip it
to `C:\Nexon\MapleStory`.

## Usage

First, read the [Gym documentation](https://gym.openai.com/docs).

Running `import maplegym` registers maplegym as an environment with
Gym. Create an environment using `gym.make('MapleGym-v0')`.

To slice gameplay into timesteps, we define a "step" as a 0.2s
pause after each action. You can change this by setting
`maplegym.conf.timestep`.

Here are the standard Gym variables:

 *  `observation` (600×800×3 uint8 array) is the game screen's RGB pixel data.
 *  `action` (uint8) is a number in `range(20)`; see [this](actions.md).
 *  `reward` (uint32) is how much your cumulative EXP plus money has increased
    in the last timestep.
 *  `done` (bool) is True if you've reached level 70.
 *  `info` (dict) usually contains diagnostic info but maplegym doesn't use it.

## Gameplay

maplegym uses an old version of the game (v.55) from about 2008. The maplegym
`Env` automatically starts a (very) small MapleStory server that handles
gameplay basics.

In general, I try to remove the metagame so the agent can focus on
fighting monsters. Thus...
    
 *  You're a Bandit.

 *  The game takes place on a small set of maps; when you reach
    the right level, you're automatically teleported to the next map.

 *  Items, AP, and SP are disabled. Your stats and skills are raised
    automatically, you automatically get new eqiups, and when you pick
    up items you're immediately compensated for the item's shop price.

 *  There's no HP or MP. When you're damaged or use a skill, you lose mesos
    equal to the amount of HP or MP lost. (Your mesos can't go under 0.)

 *  The game UI is completely removed. The screen is an image of nothing
    but you fighting monsters.

 *  All keys and mouse clicks are disabled except Control, Z, Alt, and
    the arrow keys, which correspond to Attack, Loot, Jump, and movement.
    Buffs are cast automatically, and "attack" automatically casts
    your latest feasible attacking skill (regular attack, Double Stab,
    or Savage Blow). [1]

 *  The EXP curve is rewritten to be much more lenient, since the goal
    is not an environment that takes
    forever to beat.

This reduces the game to a bitmap state, a small set of abstract actions (e.g.
"attack"), and a unified reward structure.

## Recorder

Many reinforcement learning algorithms start with a seed of past
experiences so the agent doesn't spend ages stumbling around like a
human toddler. The `maplegym.recorder.record` function lets you play
the game yourself and record your gameplay:

```python
import maplegym

for obs1, action, reward, obs2 in maplegym.recorder.record():
    # record the experience here
    pass
```

`maplegym.recorder.record` opens a MapleStory instance for you to play and
`yield`s a 4-tuple experience `(obs1, action, reward, obs2)` every timestep.
`obs1` is the 600×800×3 game screen before the timestep, `action` is the number
of the action you took, `reward` is your subsequent int32 reward, and `obs2` is
the game screen after. You can save them to a file or whatever.

## Roadmap

There's lots to do. A good chunk of maplegym is currently hardcoded in the
sense that it's not extensible. A full-fledged implementation would probably
include, for example, tools for autogenerating game files from a list of
maps/mobs/items, all jobs implemented, perhaps working quests, etc.

It all depends on how much attention this gets (I'm not hopeful), how motivated
I feel, and whether the capitalist grim reaper comes calling again soon. The
next hypothetical task is probably to turn everything into code (rather than
mysteriously patched binaries) so other people can contribute.

Pull requests and issues are warmly welcomed.

## Notes

[1] Actually, buffs are not currently autocast. There's about 20 lines of code
left to get it working. Someone open an issue to remind me.
