import math
import time
import random

from wzxml import wzxml
from util import *
import packets

SPAWN_TIME = 10

def unique_id(d):
    ret = random.randint(1, 0xffffffff)
    while ret in d:
        ret = random.randint(1, 0xffffffff)
    return ret


class Drops(dict):
    def __init__(self, player):
        self.player = player

    def add(self, drop):
        drop.objid = unique_id(self)
        self[drop.objid] = drop
        self.player.send('create_drop', drop)

    def __getitem__(self, drop_id):
        try:
            return super(Drops, self).__getitem__(drop_id)
        except:
            pass

    def __delitem__(self, drop_id):
        if drop_id in self:
            return super(Drops, self).__delitem__(drop_id)


class GameMap:
    def __init__(self, id, player):
        self.id = id
        self.mobs = {}
        self.mobs_by_index = [None for _ in xrange(len(wzxml.map[id].spawns))]
        self.drops = Drops(player)

        self.player = player
        self.player.send('player_presence', player.s.char)

        self._lastspawn = 0
        self._keep_spawning = True
        self.spawn_mobs()

    def find_alive_mob(self, mob_id):
        mob = self.mobs.get(mob_id)
        if mob and (not mob.dead):
            return mob

    def remove_mob(self, mob):
        if mob.control:
            mob.control.send('unset_mob_control', mob)

        self.player.send('kill_mob', mob)

        if mob.objid in self.mobs:
            del self.mobs[mob.objid]

        try:
            i = self.mobs_by_index.index(mob)
            self.mobs_by_index[i] = None
        except ValueError:
            pass

    def warp(self, map_id):
        self._keep_spawning = False
        if map_id > 0:
            portal = wzxml.map[map_id].portals[0]
            p = self.player
            p.s.char.update({
                'map': map_id,
                'type': 0,
                'pos': (portal.x, portal.y),
            })
            p.send('warp_map', p.s.char)
            p.s.map = GameMap(map_id, p)

    def spawn_mobs(self):
        if self._keep_spawning:
            if time.time() - self._lastspawn > SPAWN_TIME:
                self._spawn_mobs()
            stopwatch(SPAWN_TIME, self.spawn_mobs)

    def _spawn_mobs(self):
        spawns = wzxml.map[self.id].spawns
        for i, mob in enumerate(self.mobs_by_index):
            if (not mob) or mob.dead:
                mob = self.spawn_single_mob(spawns[i])
                self.mobs_by_index[i] = mob

        self._lastspawn = time.time()

    def spawn_cheat_mobs(self, n):
        def dist_from_player(spawn):
            spos = (spawn.x, spawn.cy)
            ppos = self.player.s.char.pos
            return math.sqrt(sum((float(a) - b) ** 2 for a, b in zip(spos, ppos)))

        spawn = sorted(wzxml.map[self.id].spawns, key=dist_from_player)[0]
        for i in xrange(n):
            self.spawn_single_mob(spawn)

    def spawn_single_mob(self, spawn):
        wzmob = wzxml.mob[spawn.id]

        # wtf is `type`?
        mob = adict({
            'pos': (spawn.x, spawn.cy), 'id': spawn.id, 'fh': spawn.fh,
            'type': 2, 'hp': wzmob.hp, 'mp': wzmob.mp, 'exp': wzmob.exp,
            'map': self, 'dead': False, 'control': self.player,
            'objid': unique_id(self.mobs),
        })

        self.mobs[mob.objid] = mob
        self.player.send('spawn_mob', mob)
        self.player.send('set_mob_control', mob, True)
        return mob

    def find_promising_footholds(self, x, y, ceiling):
        for fh in wzxml.map[self.id].footholds:
            if (fh.x1 <= x < fh.x2) or (fh.x2 <= x < fh.x1):
                slope = float(fh.y1 - fh.y2) / (fh.x1 - fh.x2)
                y = slope * (x - fh.x1) + fh.y1
                if y > ceiling:
                    yield y

    def find_nearest_foothold(self, x, y):
        floors = list(self.find_promising_footholds(x, y, y - 100))
        if floors:
            return int(sorted(floors)[0])
