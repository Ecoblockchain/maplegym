@serv.on(0x51)
def on_use_skill(client, packet):
    char = client.s.char
    skill_id = decode4(packet[4:])

    slevel = char.skills.levels.get(skill_id, 0)
    if slevel > 0:
        wzskill = wzxml.skill[skill_id].levels[slevel - 1]
        if skills.use_skill(client, skill_id, wzskill):
            take_mesos(client, wzskill.hp + wzskill.mp)
            client.send('set_mp', char)


@serv.on(0x4e)
def on_end_skill(client, packet):
    skills.end_skill(client, decode4(packet))


@serv.on(0x75)
def on_change_keymap(client, packet):
    client.send('send_keymap', client.s.char)  # set keymap back to what it was
    mutil.notify(client, 'Please do not modify the key settings.')

    '''
    keymap = list(client.s.char.keymap)
    count = decode4(packet[4:])

    for i in xrange(count):
        index, key = unpack('44', packet[8 + i * 9:])
        key = 0 if key % 0xff == 0 else key
        print 'setting %d to %d' % (index, key)
        keymap[index] = key

    client.s.char.keymap = keymap
    '''


@serv.on(0x2f)
def on_change_map(client, packet):
    if client.s.char.map == 20001:
        is_alive, portalname = unpack('4s', packet[1:])
        if is_alive and portalname == 'out00':
            client.s.char.vars.starttime = time.time()
            client.s.map.warp(50000)


@serv.on(0x2c)
def on_chat(client, packet):
    commands.handle_command(client, decodes(packet))

###############
# commands.py #
###############

import traceback

import conf
import mapleutil as mutil
import levels
from gamemap import GameMap
from util import autocast

handlers = {}


def handler(name):
    def decor(f):
        handlers[name] = f
        return f
    return decor
    

def handle_command(client, message):
    parts = message.split()
    func = handlers.get(parts[0])
    if func:
        try:
            func(client, *parts[1:])
        except TypeError:
            mutil.notify(client, 'Bad command.')
            traceback.print_exc()
        except:
            traceback.print_exc()


@handler('reset')
def handle_reset(client):
    levels.reset_character(client)


vartypes = {
    'exprate': float,
    'droprate': float,
    'mesorate': float,
}


@handler('set')
def handle_set(client, var, value):
    typecast = vartypes.get(var)
    if not typecast:
        mutil.notify(client, '%s is not a valid variable.' % var)

    try:
        value = typecast(value)
    except:
        msg = 'Variable %s is not of type %s.' % (var, typecast.__name__)
        mutil.notify(client, msg)
        return

    cvars = client.s.char.vars
    cvars[var] = value
    mutil.notify(client, '`%s` was set to: %s' % (var, value))

    if cvars.livemode:
        cvars.livemode = False
        mutil.notify(client, 'Livemode has been cancelled.')


@handler('livemode')
def handle_start_livemode(client):
    char = client.s.char

    char.exp = 0
    char.level = 1
    char.vars.update({
        'livemode': True,
        'exprate': 1,
        'droprate': 1,
        'mesorate': 1,
    })

    levels.levelup(client, (1,), is_natural=False)
    mutil.notify(
        client,
        ('Ok, we\'ve reset your character and set all rates to 1. '
         'Go through the portal to start the timer.'),
        ('If you change any variables during the run, livemode will '
         'be turned off.')
    )

@handler('spawn')
def handle_spawn(client, amount):
    client.s.map.spawn_cheat_mobs(int(amount))
    mutil.notify(client, 'Ok, have fun.')
