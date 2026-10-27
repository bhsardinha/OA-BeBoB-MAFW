#!/usr/bin/python
# slprops.py -- write MA* properties to the official driver's device entry on the SL rig, the way the panel does
# (IORegistryEntrySetCFProperties). Used to restore the rig's saved settings after a factory-reset test.
# Python 2.6 + PyObjC (10.6). usage:
#   python slprops.py set KEY INT                 one 32-bit integer property
#   python slprops.py restore PLIST GUID          every MA* key of PLIST[GUID] except read-only/meta ones
#   python slprops.py setarr KEY v1,v2,...        an array of 32-bit integers (2026-10-26)
import sys, ctypes, objc
from Foundation import NSDictionary, NSNumber

iokit = ctypes.cdll.LoadLibrary('/System/Library/Frameworks/IOKit.framework/IOKit')
iokit.IOServiceMatching.restype = ctypes.c_void_p
iokit.IOServiceMatching.argtypes = [ctypes.c_char_p]
iokit.IOServiceGetMatchingService.restype = ctypes.c_uint
iokit.IOServiceGetMatchingService.argtypes = [ctypes.c_uint, ctypes.c_void_p]
iokit.IORegistryEntrySetCFProperties.restype = ctypes.c_int
iokit.IORegistryEntrySetCFProperties.argtypes = [ctypes.c_uint, ctypes.c_void_p]
SKIP = ('MAMomentarySwitchLatent', 'MABytesPerCycle', 'MAEnableAggregateEngine', 'MAEnableStereoPairEngines', 'MASampleRate')

# 2026-10-26: any model -- the first device class of the official driver that is present (FW410 added for capture 103)
CLASSES = ('com_m_audio_FWAudiophileDevice', 'com_m_audio_FW410Device', 'com_m_audio_FW1814Device')

def device():
    for c in CLASSES:
        s = iokit.IOServiceGetMatchingService(0, iokit.IOServiceMatching(c))
        if s: return s
    sys.exit('no official M-Audio device entry (%s)' % ', '.join(CLASSES))

def write(svc, d):
    r = iokit.IORegistryEntrySetCFProperties(svc, objc.pyobjc_id(d))
    print '%-28s -> 0x%x' % (d.allKeys()[0], r & 0xffffffff)

if __name__ == '__main__':
    svc = device()
    if sys.argv[1] == 'set':
        write(svc, NSDictionary.dictionaryWithObject_forKey_(NSNumber.numberWithInt_(int(sys.argv[3], 0)), sys.argv[2]))
    elif sys.argv[1] == 'setarr':
        from Foundation import NSArray
        a = NSArray.arrayWithArray_([NSNumber.numberWithInt_(int(v, 0)) for v in sys.argv[3].split(',')])
        write(svc, NSDictionary.dictionaryWithObject_forKey_(a, sys.argv[2]))
    elif sys.argv[1] == 'restore':
        allp = NSDictionary.dictionaryWithContentsOfFile_(sys.argv[2])[sys.argv[3]]
        for k in sorted(allp.keys()):
            if k.startswith('MA') and k not in SKIP:
                write(svc, NSDictionary.dictionaryWithObject_forKey_(allp[k], k))
