from util import *
from wzxml import wzxml
import functools
from ctypes import c_uint16

_templates = {}


def call(fn, *a, **kw):
    return _templates[fn](*a, **kw)


def template(f):
    _templates[f.__name__] = f
    return f


@template
def login_success(username):
    return pack(0x00, '42412s44b4',
                0, 0, 0x49792a, 0, 0x6504, username, 0, 0,
                '\x00\x00\x00\xA6\xB8\x9C\x2B\x4C\xC7\x01', 0)


@template
def login_error():
    return pack(0x00, '24', 4, 0)


@template
def login_done():
    return pack(0x0d, '1', 0)


@template
def show_world(world, id):
    fmt = '1s122121'
    args = [id, world.name, 0, 0, 100, 100, 0, len(world.channels)]

    for i, chan in enumerate(world.channels):
        fmt += 's412'
        args += [chan.name, chan.population, id, i]

    return pack(0x05, fmt, *args)


@template
def show_worlds_end():
    return pack(0x05, '1', 0xff)


@template
def select_world_ack():
    return pack(0x12, '2', 0)


def char_stats(char):
    return pack_raw(
        '4b11144b1222222222224241',
        0xdeadbeef, pad_string(char.name, 12), 0, char.gender, char.skin,
        char.eyes, char.hair, ('\x00' * 24), char.level, char.job,
        char.stats.str, char.stats.dex, char.stats.int, char.stats.luk,
        char.hp, char.stats.hp, char.mp, char.stats.mp,
        char.ap, char.sp, char.exp, char.stats.fame, char.map, 0
    )


def char_equips(char):
    equips = char.inventory.equipped.values()
    return pack_raw('14' * len(equips), *flatten((x.type, x.id) for x in equips))


@template
def show_character(char):
    return pack(0x13, '11b11414b2b', 0, 1, char_stats(char), char.gender,
                char.skin, char.eyes, 1, char.hair, char_equips(char),
                -1, 25 * '\x00')


@template
def launch_character(char, server):
    ip, port = server
    ip1, ip2, ip3, ip4 = map(int, ip.split('.'))

    return pack(0x04, '211112441', 0, ip1, ip2, ip3, ip4, port, 0xdeadbeef, 0, 0)


def item_equip(equip, equipped, drop=False):
    return pack_raw(
        ('2' if drop else '1') + '142b11' + ('2' * 15) + 'b2',
        (-equip.type if drop else (equip.type if equipped else equip.pos)),
        0x01, equip.id, (equip.scrolls if equipped else 0),
        '\x80\x05\xBB\x46\xE6\x17\x02', equip.slots, equip.scrolls,
        equip.str, equip.dex, equip.int, equip.luk, equip.hp, equip.mp,
        equip.watk, equip.matk, equip.wdef, equip.mdef, equip.acc, equip.avoid,
        equip.hands, equip.speed, equip.jump, '\x00' * 10, 0
    )


@template
def give_equip(equip):
    return pack(0x18, '1111b', 1, 1, 0, 1, item_equip(equip, True, drop=True))


@template
def remove_equip(equip):
    return pack(0x18, '1111221', 1, 1, 2, 1, -equip.type, 0, 1)


def item_equips(equips, equipped):
    return ''.join(item_equip(e, equipped) for e in equips)


def is_item_star(id):
    return (id / 10000 == 207)


def item_nonequip(item):
    ret = pack_raw('1142b24', item.pos, 0x02, item.id, 0,
                   '\x80\x05\xBB\x46\xE6\x17\x02', item.quantity, 0)

    if is_item_star(item.id):
        ret += '\x02\x00\x00\x00\x54\x00\x00\x34'
    return ret


def item_nonequips(items):
    return ''.join(item_nonequip(it) for it in items)


def char_items(char):
    equipped_cash = ''
    equipped = item_equips(char.inventory.equipped.values(), True)
    equips = ''
    otheritems = '\x00'.join('' for i in xrange(2, 5+1))

    return '\x00'.join((equipped, equipped_cash, equips, otheritems)) + '\x00'


def char_skills(char):
    levels = char.skills.levels
    skills = ''.join(pack_raw('44', sid, slevel) for sid, slevel
                     in levels.iteritems())

    return pack_raw('2b', len(levels), skills)


@template
def char_connect(char, server_time):
    return pack(0x4e, '4b2b14bbbbb8',
                1, '\x01\x01\x85\x3D\x4B\x11\xF4\x83\x6B\x3D\xBA\x9A\x4F\xA1',
                -1, char_stats(char), 0x14, char.inventory.mesos,
                '\x64' * 5, char_items(char), char_skills(char), '\x00' * 14,
                '\xff\xc9\x9a\x3b' * 15, server_time)


@template
def spawn_mob(mob, is_new=True):
    offset = 1 if is_new else 20
    return pack(0x97, '4144221222',
                mob.objid, 1, mob.id, 0, mob.pos[0], mob.pos[1] - offset,
                mob.type, 0, mob.fh, (-2 if is_new else -1))


@template
def char_move(char, movement_data):
    return pack(0x85, '44b', 0xdeadbeef, 0, movement_data)


@template
def set_mob_control(mob, is_new=True):
    offset = 1 if is_new else 20
    return pack(0xa5, '14144221222', 1, mob.objid, 1, mob.id, 0,
                mob.pos[0], mob.pos[1] - offset, mob.type, 0, mob.fh, -1)


@template
def unset_mob_control(mob):
    return pack(0xa5, '14', 0, mob.objid)


@template
def move_mob_self(mob, unknown):
    return pack(0x9d, '4214', mob.objid, unknown, 0, mob.mp)


@template
def move_mob_others(mob, movement_data):
    return pack(0x98, '4141b', mob.objid, 0, 0xff, 0, movement_data)


@template
def player_presence(char):
    return pack(0x66, '4sbb11414b1b1b4422142b',
                0xdeadbeef, char.name, '\x00' * 8,
                '\x00' * 8, char.gender, char.skin, char.eyes, 1, char.hair,
                char_equips(char), -1, char_equips(char), -1, 
                '\x00' * 20, 0, 0, char.pos[0],
                char.pos[1], char.type, 0, 1, '\x00' * 16)


@template
def show_mob_hp_percent(mob):
    perc = max(mob.hp * 100 / wzxml.mob[mob.id].hp, 0)
    return pack(0x99, '41', mob.objid, perc)


@template
def kill_mob(mob):
    return pack(0xa6, '41', mob.objid, 1)


@template
def add_exp(amount):
    return pack(0x32, '1144441', 3, 1, amount, 0, 0, 0, 0)


@template
def set_exp(char):
    return pack(0x23, '2224', 0, 0, 1, char.exp)


@template
def show_login_screen():
    return pack(0x13, '1', 1)


@template
def level_up_self(char):
    return pack(0x23, '2221222224',
                0, 0x7c10, 1, char.level, char.hp, char.stats.hp,
                char.mp, char.stats.mp, char.ap, char.exp)


@template
def level_up_others(char):
    return pack(0x86, '41', 0xdeadbeef, 0)


@template
def set_hp(char):
    return pack(0x23, '12212', 0, 0, 4, 0, char.hp)


@template
def set_mp(char):
    return pack(0x23, '12212', 0, 0, 16, 0, char.mp)


@template
def create_drop(drop):
    is_meso = (drop.mesos > 0)
    drop_data = (drop.mesos if is_meso else drop.item)

    return pack(0xb9, '1414412242221b',
                1, drop.objid, 1 if is_meso else 0,
                drop_data, drop.mob.objid, 0, drop.pos[0], drop.pos[1],
                0, drop.mob.pos[0], drop.mob.pos[1], 0, 0,
                '' if is_meso else '\x80\x05\xBB\x46\xE6\x17\x02\x00')


@template
def set_stat(char, stat):
    id = 0x4000 + {'str': 0x40, 'dex': 0x80, 'int': 0x100, 'luk': 0x200}[stat]
    return pack(0x23, '2422', 1, id, char.stats[stat], 0)


@template
def loot_mesos_note(mesos):
    return pack(0x32, '1142', 0, 1, mesos, 0)


@template
def quest_mesos_note(mesos):
    return pack(0x32, '14', 5, mesos)


@template
def pickup_drop(drop, char):
    return pack(0xba, '144', 2, drop.objid, 0xdeadbeef)


@template
def set_mesos(char):
    return pack(0x23, '2224', 1, 0, 4, char.inventory.mesos)


@template
def show_player_damage(char, mob, damage):
    return pack(0x8a, '4144214',
                0xdeadbeef, -1, damage, mob.id, 1, 0, damage)


@template
def update_player(char):
    return pack(0x93, '4111414b1b1b',
                0xdeadbeef, 1, char.gender, char.skin, char.eyes, 1, char.hair,
                char_equips(char), -1, char_equips(char), -1,
                '\x00' * 19)

@template
def advance_job_self(char):
    return pack(0x23, '242', 0, 0x20, char.job)


@template
def advance_job_others(char):
    return pack(0x86, '41', 0xdeadbeef, 8)


@template
def remove_player(char):
    return pack(0x71, '4', 0xdeadbeef)


@template
def warp_map(char):
    return pack(0x4e, '424121b',
                0, 0, char.map, char.map, char.hp, 0,
                '\xff' * 8)


@template
def raise_skill(skillid, level):
    return pack(0x2f, '124441', 1, 1, skillid, level, 0, 1)


@template
def send_message(message, type=6):
    return pack(0x2d, '1s', type, message)


@template
def use_skill(skill_id, time, types, vals):
    fmt = ('1' * len(types)) + ('244' * len(vals)) + '21'
    args = list(types)

    for val in vals:
        args += [val, skill_id, time]

    args += [0, 0]
    return pack(0x3b, fmt, *args)


@template
def end_skill(skill_id, types):
    args = list(types) + [0]
    return pack(0x24, '1' * len(args), *args)


@template
def send_keymap(char):
    return pack(0xf7, '1' + ('41' * len(char.keymap)),  0,
                *flatten((x, 0) for x in char.keymap))
