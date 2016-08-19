import random
import time
import ctypes

import packets
import server
import commands
import skills
import levels
import conf
import mapleutil as mutil
from util import *
from wzxml import wzxml
from gamemap import GameMap

serv = server.MapleServer('0.0.0.0', 8485)
serv.s.player = None

_player_login_event = None


def get_server_time():
    return 0  # todo if it's important


@serv.on('DISCONNECT')
def on_player_disconnect(client, _):
    m = client.s.get('map')
    if m:
        m.warp(0)

    serv.s.player = None


@serv.on(0x14)
def on_player_connect(client, packet):
    if decode4(packet) != 0xdeadbeef:
        return client.close()

    serv.s.player = client
    char = client.s.char = mutil.create_character()

    _player_login_event.set()

    client.send('char_connect', char, get_server_time())
    client.send('send_keymap', char)
    client.s.map = GameMap(char.map, client)


def take_mesos(client, amount):
    char = client.s.char
    char.inventory.mesos -= amount
    char.inventory.mesos = max(0, char.inventory.mesos)

    client.send('quest_mesos_note', -amount)
    client.send('set_mesos', char)


def show_player_damage(client, mob_id, damage):
    gmap = client.s.map
    mob = gmap.find_alive_mob(mob_id)
    if mob:
        client.send('show_player_damage', client.s.char, mob, damage)


@serv.on(0x2a)
def on_damage_player(client, packet):
    damage = decode4(packet[5:])
    if damage > 0:
        take_mesos(client, damage)

    if len(packet) >= 17:
        show_player_damage(client, decode4(packet[13:]), damage)


@serv.on(0x35)
def on_character_move(client, packet):
    fifth = decode1(packet[5])
    size = fifth * 14 + 1

    char = client.s.char
    x, y = unpack('22', packet[:-4])
    char.pos = (x, y)

    itype = 5 + 14 * (fifth - 1) + 12
    if 0 <= itype < len(packet):
        char.type = decode1(packet[itype])

    client.send('char_move', char, packet[5:5+size])


def vary_mesos(amount):
    fifth = amount / 5
    return amount + random.randint(-fifth, fifth)


def generate_rewards(mob, droprate):
    for item in wzxml.reward[mob.id].rewards:
        if random.random() < item.prob * droprate:
            yield item


def generate_mob_drops(char, mob):
    for item in generate_rewards(mob, char.vars.droprate):
        mesos = int((item.get('money') or 0) * char.vars.mesorate)
        yield adict({'item': item.get('item') or 0,
                     'mob': mob, 'mesos': vary_mesos(mesos)})


def drop_offsets():
    offset = 0
    while True:
        yield offset
        offset = -offset
        if offset >= 0:
            offset += 25


def generate_drops(client, char, mob):
    for offset, drop in zip(drop_offsets(), generate_mob_drops(char, mob)):
        x, y = (mob.pos[0] + offset), mob.pos[1]
        floor = client.s.map.find_nearest_foothold(x, y)
        if floor is not None:
            drop.pos = (x, floor)
            yield drop


def get_drop_value(drop, mesorate):
    def get_item_price(itemid):
        table = wzxml.equip if itemid / 1000000 == 1 else wzxml.item
        return table[itemid].price

    return drop.mesos or int(get_item_price(drop.item) * mesorate)


def drop_mob_rewards(client, mob):
    char = client.s.char
    for drop in generate_drops(client, char, mob):
        if get_drop_value(drop, char.vars.mesorate) > 1:
            client.s.map.drops.add(drop)


def kill_mob(client, mob):
    mob.map.remove_mob(mob)

    exp = int(mob.exp * client.s.char.vars.exprate)
    warped = levels.give_player_exp(client, exp)
    if not warped:
        drop_mob_rewards(client, mob)


@serv.on(0x59)
def on_damage_mob(client, packet):
    strikes, skill_id = unpack('14', packet[1:])
    mobcount, hits = strikes / 16, strikes % 16

    if skill_id > 0:
        char = client.s.char
        slevel = char.skills.levels.get(skill_id, 0)
        if slevel > 0:
            wzskill = wzxml.skill[skill_id].levels[slevel - 1]
            take_mesos(client, wzskill.hp + wzskill.mp)
            client.send('set_mp', char)

    for i in xrange(mobcount):  # number of mobs
        offset = 14 + i * (22 + 4 * (hits - 1))
        mob = client.s.map.find_alive_mob(decode4(packet[offset:]))
        if mob:
            for k in xrange(hits):  # number of hits each
                # todo: check if user is hacking
                dmg = decode4(packet[32 + i * (22 + 4 * (hits - 1)) + (k * 4):])
                mob.hp -= dmg

            client.send('show_mob_hp_percent', mob)
            if mob.hp <= 0:
                kill_mob(client, mob)


@serv.on(0x89)
def on_loot_item(client, packet):
    char, gmap = client.s.char, client.s.map

    drop = gmap.drops[decode4(packet[9:])]
    if drop is None:
        return

    value = get_drop_value(drop, char.vars.mesorate)
    char.inventory.mesos += value

    client.send('pickup_drop', drop, char)
    client.send('set_mesos', client.s.char)

    del gmap.drops[drop.objid]


@serv.on(0x9d)
def on_control_mob(client, packet):
    mob = client.s.map.find_alive_mob(decode4(packet))
    if mob and mob.control is client:
        mob.type = decode1(packet[-12:])
        mob.pos = [ctypes.c_int16(x).value for x in unpack('22', packet[-4:])]

        client.send('move_mob_self', mob, decode2(packet[4:]))


def run(event):
    global _player_login_event

    _player_login_event = event
    serv.run()
