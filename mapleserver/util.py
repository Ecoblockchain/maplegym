import threading
import functools
import traceback
import collections
import ctypes
import struct
import attrdict
import functools
import colorama

colorama.init()


def threadify(fn, *a, **kw):
    thread = threading.Thread(target=fn, args=a, kwargs=kw)
    thread.daemon = True
    thread.start()
    return thread


def xor_bytes(x, y):
    return bytearray([a ^ b for a, b in zip(x, y)])


def stopwatch(sec, fn, *a, **kw):
    timer = threading.Timer(sec, fn, args=a, kwargs=kw)
    timer.start()
    return timer


def repeat(sec, fn, *a, **kw):
    def again():
        fn(*a, **kw)
        repeat(sec, fn, *a, **kw)
    stopwatch(sec, again)


def format_bytes(seq):
    return ' '.join(('%02X' % ord(x) for x in seq))


def print_bytes(seq):
    print format_bytes(seq)


fmt_types = {'1': 'B', '2': 'H', '4': 'L', '8': 'Q'} 
ctype_casts = {
    1: ctypes.c_uint8, 2: ctypes.c_uint16, 4: ctypes.c_uint32,
    8: ctypes.c_uint64,
}


class PackError(Exception):
    def __init__(self):
        super(PackError, self).__init__('invalid pack format: %s' % fmt)


def pack_raw(fmt, *args):
    def build():
        for ch, val in zip(fmt, args):
            if ch in fmt_types:
                cast = ctype_casts[int(ch)]
                yield struct.pack('<' + fmt_types[ch], cast(val).value)
            elif ch == 's':
                size = len(val)
                yield struct.pack('<H', size)
                yield struct.pack('<%ds' % size, val)
            elif ch == 'b':
                yield struct.pack('<%ds' % len(val), val)
            else:
                raise PackError(fmt)
    return ''.join(build())


def pack(opcode, fmt, *args):
    return pack_raw('2' + fmt, opcode, *args)


def _unpack(*a):
    return struct.unpack(*a)[0]


def unpack(fmt, s):
    for ch in fmt:
        if ch in fmt_types:
            size = int(ch)
            yield _unpack('<' + fmt_types[ch], s[:size])
        elif ch == 's':
            size, s = _unpack('<H', s[:2]), s[2:]
            yield _unpack('<%ds' % size, s[:size])
        else:
            raise PackError(fmt)
        s = s[size:]


def unpack1(fmt, s):
    return unpack(fmt, s).next()


encode1 = functools.partial(pack, '1')
encode2 = functools.partial(pack, '2')
encode4 = functools.partial(pack, '4')
encodes = functools.partial(pack, 's')
decode1 = functools.partial(unpack1, '1')
decode2 = functools.partial(unpack1, '2')
decode4 = functools.partial(unpack1, '4')
decodes = functools.partial(unpack1, 's')


def pad_string(s, n):
    return s + ''.join('\x00' * (n - len(s)))


adict = attrdict.AttrMap
adefault = attrdict.AttrDefault


def segregate(xs, fn):
    yes, no = [], []
    for x in xs:
        (yes if fn(x) else no).append(x)
    return yes, no


def bucket(xs, fn):
    ret = {}
    for x in xs:
        ret.setdefault(fn(x), []).append(x)
    return ret


# only goes exactly 1 level deep
def flatten(l):
    return (item for sublist in l for item in sublist)


def cprint(color, s):
    print getattr(colorama.Fore, color) + str(s) + colorama.Fore.RESET


class keydefaultdict(collections.defaultdict):
    def __missing__(self, key):
        if self.default_factory is None:
            raise KeyError(key)

        ret = self[key] = self.default_factory(key)
        return ret


def copy_adict(x):
    return adict(dict(x))


def safecall(f):
    def _(*a, **kw):
        try:
            return f(*a, **kw)
        except:
            traceback.print_exc()
    return _


def autocast(s):
    for cast in (int, float, str):
        try:
            return cast(s)
        except:
            pass
    raise TypeError('what the hell did you send me')
