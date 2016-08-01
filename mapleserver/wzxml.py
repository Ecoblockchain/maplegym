import bs4
import os.path

import conf
from util import adict, segregate, autocast


def read_soup(filename):
    with open(filename) as f:
        return bs4.BeautifulSoup(f.read(), 'html.parser')


def dictify(node):
    def run():
        for child in node.contents:
            if not isinstance(child, bs4.element.NavigableString):
                yield child.name, autocast(child.text)
    return dict(run())


class WzXmlStore:
    def __init__(self, path):
        self._path = path
        print '[maplegym] Reading WZXML files...'

        self.map = self.read('map', conf.hunting_maps.values(), self.read_map)

        mob_ids = self.get_mob_ids()
        self.mob = self.read('mob', mob_ids, dictify)
        self.reward = self.read('reward', mob_ids, self.read_reward)

        item_ids, equip_ids = self.get_item_ids()
        self.item = self.read('item', item_ids, dictify)
        self.equip = self.read('equip', equip_ids, dictify)

        self.skill = self.read('skill', self.get_skill_ids(), self.read_skill)

        # remove references to objects we don't have xml files for
        self.filter_spawns()
        self.filter_rewards()

    def get_skill_ids(self):
        for fname in os.listdir(os.path.join(self._path, 'skill')):
            if fname.endswith('.xml'):
                skill_id = int(os.path.splitext(fname)[0])
                job = skill_id / 10000
                if job > 0 and job in conf.jobs.values():
                    yield skill_id

    def filter_spawns(self):
        def found(spawn):
            return spawn.id in self.mob and spawn.id in self.reward

        for mapid, mapdata in self.map.iteritems():
            mapdata.spawns = filter(found, mapdata.spawns)

    def filter_rewards(self):
        def found(item):
            if 'item' not in item:
                return True
            if item.item in self.item:
                return True
            if item.item in self.equip:
                return True

        for mob, rewards in self.reward.iteritems():
            rewards.rewards = filter(found, rewards.rewards)

    def get_item_ids(self):
        items = set()
        equips = set(sum(conf.bandit_equips.values(), []))

        def is_equip(x):
            return (x / 1000000 == 1)

        for m in self.reward.values():
            rewards = (x.item for x in m.rewards if 'item' in x)
            equip_ids, item_ids = segregate(rewards, is_equip)
            items |= set(item_ids)
            equips |= set(equip_ids)

        return items, equips

    def get_mob_ids(self):
        ret = set()
        for m in self.map.values():
            ret |= set(x.id for x in m.spawns)
        return ret

    def read(self, wztype, ids, cb):
        def run():
            for id in ids:
                path = os.path.join(self._path, wztype, '%d.xml' % id)
                if os.path.isfile(path):
                    yield id, adict(cb(read_soup(path).find(wztype)))

        return adict(run())

    def read_map(self, node):
        return {
            'returnmap': int(node.find('returnmap').string),
            'spawns': map(dictify, node.find_all('mob')),
            'portals': map(dictify, node.find_all('portal')),
            'footholds': map(dictify, node.find_all('foothold')),
        }

    def read_skill(self, node):
        return {'levels': map(dictify, node.find_all('level'))}

    def read_reward(self, node):
        return {'rewards': map(dictify, node.select('items > item'))}


wzxml_folder = os.path.join(os.path.dirname(__file__), 'wzxml')
wzxml = WzXmlStore(wzxml_folder)
