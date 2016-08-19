import threading
import time
from util import threadify
import loginserver
import gameserver


class MasterServer:
    def start(self):
        self._player_login_event = threading.Event()

        self.th1 = threadify(loginserver.serv.run)
        self.th2 = threadify(gameserver.run, self._player_login_event)

        print '[maplegym] LOGIN started on port 8484.'
        print '[maplegym] SCANIA-1 started on port 8485.'

    def wait_login(self):
        self._player_login_event.wait()

    @property
    def state(self):
        return gameserver.serv.s

    def stop(self):
        loginserver.serv.stop()
        gameserver.serv.stop()
        self.th1.join()
        self.th2.join()


def start():
    serv = MasterServer()
    serv.start()
    return serv
