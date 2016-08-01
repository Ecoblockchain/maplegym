import conf
import time

from util import stopwatch
import mapleutil as mutil
import skills
from wzxml import wzxml
from gamemap import GameMap


def apply_stats_change(stats, action, change):
    for k, v in change.iteritems():
        if action == 'add':
            stats[k] += v
        elif action == 'set':
            stats[k] = v


def change_stats(client, levels):
    char = client.s.char
    for level in levels:
        for action, change in conf.get_stats_change(level).iteritems():
            apply_stats_change(char.stats, action, change)


def change_equips(client, levels):
    char, pipeline = client.s.char, []

    for level in levels:
        if level == 1:
            for equip in char.inventory.equipped.values():
                client.send('remove_equip', equip)
        for type, equip in mutil.make_equip_set(level).iteritems():
            char.inventory.equipped[type] = equip
            client.send('give_equip', equip)


def send_levelup(client):
    char = client.s.char

    client.send('level_up_others', char)
    client.send('level_up_self', char)
    for x in ('str', 'dex', 'int', 'luk'):
        client.send('set_stat', char, x)


def change_job(client, levels):
    char = client.s.char
    oldjob = char.job

    for level in levels:
        job = conf.jobs.get(level)
        if job is not None:
            char.job = job

    if oldjob != char.job:
        client.send('advance_job_self', char)


def change_skills(client, levels):
    skills = client.s.char.skills.levels

    for level in levels:
        if level == 1:
            for sid in skills:
                skills[sid] = 0
        else:
            build = conf.skill_build.get(level)
            for sid, amount in (build or {}).iteritems():
                skills.setdefault(sid, 0)
                skills[sid] += amount

    for sid, slevel in skills.iteritems():
        client.send('raise_skill', sid, slevel)


def change_map(client, levels):
    destination = None
    for level in levels:
        mapid = conf.hunting_maps.get(level)
        if mapid:
            destination = mapid

    if destination is not None:
        client.s.map.warp(destination)
        return True


def format_time(s):
    m, s = divmod(s, 60)
    h, m = divmod(m, 60)

    timestr = '%d minutes, %d seconds' % (m, s)
    if h > 0:
        timestr = '%d hours, %s' % (h, timestr)
    return timestr


def handle_rebirth(client, is_natural):
    char = client.s.char

    char.inventory.mesos = 0
    client.send('set_mesos', char)

    active = list(char.skills.active)
    for skill_id in active:
        skills.end_skill(client, skill_id)

    if is_natural:
        seconds = time.time() - char.vars.starttime
        mutil.notify(client, 'You finished in %s.' % format_time(seconds))

        runs = list(char.runs)
        runs.append({'time': seconds, 'livemode': char.vars.livemode})
        char.runs = runs

        char.vars.starttime = time.time()


def change_keymap(client, levels):
    patch = {}
    for level in levels:
        patch.update(conf.keymap_changes.get(level, {}))

    if patch:
        char = client.s.char
        keymap = list(char.keymap)

        for k, v in patch.iteritems():
            keymap[k] = v

        char.keymap = keymap
        client.send('send_keymap', char)


def levelup(client, levels, is_natural=True, warp=True):
    def run():
        change_stats(client, levels)
        change_skills(client, levels)
        change_keymap(client, levels)
        change_equips(client, levels)

        send_levelup(client)
        change_job(client, levels)

        if 1 in levels:
            handle_rebirth(client, is_natural)

    if warp and change_map(client, levels):
        stopwatch(0.8, run)
        return True
    else:
        run()


def add_levels(char):
    while char.exp >= conf.exp_table[char.level - 1] and char.level < 70:
        char.exp -= conf.exp_table[char.level - 1]
        char.level += 1
        yield char.level
        if char.level >= 70:
            char.exp = 0
            break


def give_player_exp(client, amount):
    char = client.s.char
    warped = False

    char.exp += amount
    # client.send('add_exp', amount)
    levels = list(add_levels(char))
    client.send('set_exp', char)

    if levels:
        return levelup(client, levels)


def reset_character(client, warp=True):
    client.s.char.level = 1
    client.s.char.exp = 1

    levelup(client, (1,), is_natural=False, warp=warp)
    send_levelup(client)

    mutil.notify(client, 'Your session has been reset.')
