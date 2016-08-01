from wzxml import wzxml
import conf
import server
from util import *
import mapleutil as mutil

serv = server.MapleServer('0.0.0.0', 8484)


@serv.on(0x1b)
def on_login_auth(client, packet):
    username = decodes(packet)
    client.send('login_success', username)


@serv.on(0x03)
def on_login_done(client, packet):
    client.send('login_done')


@serv.on(0x02)
@serv.on(0x18)
def on_user_authed(client, packet):
    client.send('show_world', conf.world, 0)
    client.send('show_worlds_end')

    char = mutil.create_character()
    client.send('show_character', char)
    client.send('launch_character', char, ('127.0.0.1', 8485))


@serv.on(0x1a)
def on_show_login_screen(client, packet):
    client.send('show_login_screen')
