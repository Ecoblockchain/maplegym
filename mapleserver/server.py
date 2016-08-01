import socket
from multiprocessing import Lock
import packets
from util import *

# GMS v.55
GAME_LOCALE = 0x08
GAME_VERSION = 55

print_lock = Lock()


class MapleClient:
    def __init__(self, server, conn, addr):
        self.conn = conn
        self.addr = addr
        self.server = server
        self.s = adict()
        self.send_lock = Lock()

    def close(self):
        self.conn.close()
        self.server.remove_client(self)

    def _sendbytes(self, s):
        try:
            return self.conn.sendall(s)
        except:
            raise IOError

    def print_packet(self, packet, handshake):
        opcode = decode2(packet[:2])
        if opcode != 0x9d:
            opcode = 'HELO' if handshake else ('%04X' % opcode)
            data = format_bytes(packet if handshake else packet[2:])
            with print_lock:
                # cprint('GREEN', '[SEND] %s %s' % (opcode, data))
                pass

    def sendbytes(self, packet, handshake=False):
        with self.send_lock:
            self.print_packet(packet, handshake)
            try:
                self._sendbytes(pack_raw('2', len(packet)))
                self._sendbytes(packet)
            except IOError:
                self.close()

    def send(self, fn, *a, **kw):
        self.sendbytes(packets.call(fn, *a, **kw))

    def recvall(self, n):
        buf = ''
        while len(buf) < n:
            part = self.conn.recv(n - len(buf))
            if not part:
                raise IOError
            buf += part
        return buf

    def recv(self):
        try:
            self.recvall(2)  # throwaway
            packet = self.recvall(decode2(self.recvall(2)))
            return decode2(packet[:2]), packet[2:]
        except IOError:
            self.close()

    def handshake(self):
        self.sendbytes(pack_raw('22441', GAME_VERSION, 0, 0, 0, GAME_LOCALE),
                       handshake=True)


class MapleServer:
    def __init__(self, host, port):
        self._handlers = {}
        self._threads = {}
        self.host = (host, port)
        self._stop = threading.Event()
        self.s = adict()

    def on(self, opcode):
        def inner(fn):
            self._handlers[opcode] = fn
            return fn
        return inner

    def remove_client(self, client):
        self._call(client, 'DISCONNECT')
        if client in self._threads:
            del self._threads[client]

    def _call(self, client, opcode, packet=None):
        fn = self._handlers.get(opcode)
        if fn:
            fn(client, packet)

    def init_listen_socket(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.bind(self.host)
        sock.listen(10)
        return sock

    def accept_clients(self):
        self.sock = self.init_listen_socket()
        while True:
            conn, addr = self.sock.accept()
            if self._stop.is_set():
                break

            client = MapleClient(self, conn, addr)
            client.handshake()
            yield client

    def stop(self):
        self._stop.set()

        # close listening socket by making accept return
        conn = socket.socket()
        conn.connect(('127.0.0.1', self.host[1]))
        conn.close()
        self.sock.close()

        # close all clients
        for client, thread in self._threads.items():
            client.close()
            thread.join()

    def run(self):
        def handle_packets(client):
            for opcode, packet in iter(client.recv, None):
                if opcode != 0x9d:
                    with print_lock:
                        # cprint('YELLOW', '[RECV] %04X %s' % (opcode, format_bytes(packet)))
                        pass
                self._call(client, opcode, packet)

        for client in self.accept_clients():
            if self._stop.is_set():
                break

            self._call(client, 'CONNECT')
            self._threads[client] = threadify(handle_packets, client)
