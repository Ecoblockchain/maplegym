from util import *
import time
from wzxml import wzxml
import conf

attrs = '''
    type pos id scrolls slots scrolls str dex int luk hp mp
    watk matk wdef mdef acc avoid hands speed jump scrolls
'''.split()

defaults = {'slots': 7, 'pos': 1}


def equipdict(obj):
    for attr in attrs:
        obj.setdefault(attr, defaults.get(attr, 0))
    return adict(obj)


def make_equip_set(level):
    def run():
        for itemid in conf.bandit_equips.get(level, []):
            eq = equipdict(adict(dict(wzxml.equip[itemid])))
            eq.id = itemid
            yield eq.type, eq
    return dict(run())


def notify(client, *messages):
    for msg in messages:
        client.send('send_message', msg)


def create_character():
    stats = copy_adict(conf.get_stats_change(1).set)
    return adict({
        'name': 'agent', 'eyes': 20000, 'hair': 30030, 'skin': 0,
        'gender': 0, 'job': conf.jobs[1], 'level': 1,
        'exp': 1, 'pos': (0, 0), 'type': 0, 'map': conf.hunting_maps[1],
        'ap': 0, 'sp': 0, 'stats': stats, 'hp': stats.hp, 'mp': stats.mp,
        'inventory': {'equipped': make_equip_set(1), 'mesos': 0},
        'skills': {'active': {}, 'levels': {}},
        'vars': {
            'starttime': time.time(), 'exprate': 20, 'mesorate': 1,
            'droprate': 1, 'livemode': False,
        },
        'runs': [], 'keymap': conf.default_keymap[:],
    })
