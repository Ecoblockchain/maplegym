# maplegym

maplegym adds [MapleStory](http://maplestory.nexon.net) to [OpenAI
Gym](https://gym.openai.com/) so you can teach programs/neural
networks to play for you.

## Installation

```
pip install gym
git clone https://github.com/brhs/maplegym
```

Move the maplegym folder into your project directory or `$PYTHONPATH` so you
can `import` it. Install maplegym's [modified MapleStory
client](https://github.com/brhs/maplegym/releases/tag/v1.0).

## Usage

First, read the [Gym documentation](https://gym.openai.com/docs).

Running `import maplegym` registers maplegym as an environment with
Gym. Create an environment using `gym.make('MapleGym-v0')`.

To slice gameplay into timesteps, we define a "step" as a 0.2s pause
after each action. Change this by setting `maplegym.conf.timestep`.

Here are the standard Gym variables:

 *  `obs` (600×800×3 uint8 array) is the game screen's RGB pixel data.
 *  `action` (uint8) is a number in `range(20)`; see [this](actions.md).
 *  `reward` (uint32) is how much your cumulative EXP plus money has increased
    in the last timestep.
 *  `done` (bool) is True if you've reached level 70.
 *  `info` (dict) usually contains diagnostic info but maplegym doesn't use it.

## Gameplay

maplegym uses an old version of the game (v.55) from about 2008. The
maplegym `Env` automatically starts a (very) small local MapleStory
server and runs the game client.

 *  You're a Bandit.
 *  The game takes place on a small set of maps; when you reach
    the right level, you're automatically teleported to the next map.
 *  Items, AP, and SP are disabled. Your stats and skills are raised
    automatically, you automatically get new eqiups, and when you pick
    up items you're immediately compensated for the item's shop price.
 *  There's no HP or MP. When you're damaged or use a skill, you lose mesos
    equal to the amount of HP or MP lost. (Your mesos can't go under 0.)
 *  The game UI is completely removed.
 *  All keys and mouse clicks are disabled except Control, Z, Alt, and
    the arrow keys, which correspond to Attack, Loot, Jump, and movement.
    Buffs are autocast, and "attack" is automatically mapped to
    your latest feasible attacking skill (regular attack, Double Stab,
    or Savage Blow). [1]
 *  The EXP curve is rewritten to be much more lenient, since the goal
    is not an environment that takes
    forever to beat.

This reduces the game to a bitmap state, a small set of abstract
actions, and a unified reward structure.

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

`maplegym.recorder.record` opens a MapleStory window and `yield`s an
experience 4-tuple after every timestep. These mean the same thing as
described described above in [Usage](#usage).  You can save them to a
file or whatever.

## Roadmap

The next milestone is to make maplegym extensible and implement
everything in code instead of mysteriously binary patches that no one
understands. There's a lot more documentation to be written.

Progress largely depends how much attention this gets (I'm not
hopeful), how motivated I feel, and whether the capitalist grim reaper
comes calling again soon. Pull requests and issues are warmly
welcomed.

## Notes

[1] Actually, buffs are not currently autocast. There's about 20 lines of code
left to get it working. Someone open an issue to remind me.
