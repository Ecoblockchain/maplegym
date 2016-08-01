from wzxml import wzxml
from util import adict, stopwatch


# ripped from titanms, idk wtf this does/means
def make_skilldata():
    weird_buff_constants = {
        4201002: ((0x08, 2, 'x'),),
        4001003: ((0x80, 1, 'speed'), (0x04, 2, 'x')),
        4201003: ((0x80, 1, 'speed'), (0x01, 2, 'jump')),
    }

    for sid, buffs in weird_buff_constants.iteritems():
        types, attrs = [0 for _ in xrange(8)], []
        for btype, index, attr in buffs:
            types[index - 1] += btype
            attrs.append(attr)
        yield sid, (types, attrs)

skilldata = dict(make_skilldata())


def use_skill(client, skill_id, wzskill):
    char = client.s.char
    if skill_id in char.skills.active:
        char.skills.active.pop(skill_id).cancel()

    data = skilldata.get(skill_id)
    if data is not None:
        types, attrs = data
        attrs = map(wzskill.get, attrs)
        client.send('use_skill', skill_id, wzskill.time * 1000, types, attrs)

        char.skills.active[skill_id] = stopwatch(
            wzskill.time, end_skill, client, skill_id, is_timer=True)

        return True


def end_skill(client, skill_id, is_timer=False):
    char = client.s.char
    if skill_id in char.skills.active:
        timer = char.skills.active.pop(skill_id)
        if not is_timer:
            timer.cancel()

    data = skilldata.get(skill_id)
    if data is not None:
        types, _ = data
        client.send('end_skill', skill_id, types)
