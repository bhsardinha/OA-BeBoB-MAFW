#!/usr/bin/python
# slhal.py -- list CoreAudio devices with UID, nominal rate and in/out channel counts (SL 10.6 rig; Python 2.6 ctypes,
# uses the 10.6-era AudioHardwareGetProperty / AudioDeviceGetProperty calls). Read-only.
import ctypes, struct
CA = ctypes.cdll.LoadLibrary('/System/Library/Frameworks/CoreAudio.framework/CoreAudio')
CF = ctypes.cdll.LoadLibrary('/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation')
def fourcc(s): return struct.unpack('>I', s)[0]
CF.CFStringGetCString.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_long, ctypes.c_uint32]
def cfstr(p):
    b = ctypes.create_string_buffer(256); CF.CFStringGetCString(p, b, 256, 0x08000100); return b.value
def hw(sel):
    size = ctypes.c_uint32(0); CA.AudioHardwareGetPropertyInfo(fourcc(sel), ctypes.byref(size), None)
    buf = (ctypes.c_uint32 * (size.value // 4))(); CA.AudioHardwareGetProperty(fourcc(sel), ctypes.byref(size), buf); return list(buf)
def dev(d, sel, isInput=0, ctype=ctypes.c_void_p):
    v = ctype(); size = ctypes.c_uint32(ctypes.sizeof(v))
    r = CA.AudioDeviceGetProperty(d, 0, isInput, fourcc(sel), ctypes.byref(size), ctypes.byref(v)); return v.value if r == 0 else None
def channels(d, isInput):
    size = ctypes.c_uint32(0)
    if CA.AudioDeviceGetPropertyInfo(d, 0, isInput, fourcc('slay'), ctypes.byref(size), None): return 0
    buf = ctypes.create_string_buffer(size.value); CA.AudioDeviceGetProperty(d, 0, isInput, fourcc('slay'), ctypes.byref(size), buf)
    n = struct.unpack_from('I', buf, 0)[0]
    return sum(struct.unpack_from('I', buf, 8 + 16 * i)[0] for i in range(n))
default_out = hw('dOut'); default_in = hw('dIn ')
for d in hw('dev#'):
    print '%-4d %-28s uid=%-42s rate=%-7s in=%-2d out=%-2d%s' % (d, cfstr(dev(d, 'lnam')), cfstr(dev(d, 'uid ')),
        dev(d, 'nsrt', 0, ctypes.c_double), channels(d, 1), channels(d, 0),
        (' defaultOut' if d in default_out else '') + (' defaultIn' if d in default_in else ''))
