#!/usr/bin/python
# slpanel.py -- drive the official M-Audio panel on the SL rig for screenshots (bench tool, Python 2.6 + PyObjC Quartz, 10.6).
# Run inside the console user's GUI session:  sudo launchctl bsexec <Panel PID> sudo -u <user> python slpanel.py <cmd>
#   windows                 list on-screen windows of the panel process (owner name "M-Audio Firewire") (id, bounds, name)
#   click X Y               left click at global screen coordinates (points)
#   front                   bring the Panel process to the front
#   shot FILE.png           save an image of the panel window only (10.6 screencapture has no -l)
#   seq FILE [SHOTDIR]      run an action list; coordinates are relative to the panel window's top-left; each action is
#                           logged as "<wall ms> <action line>" so DTrace records (slsetprops.d) can be matched to it.
#                           lines: click LABEL X Y | drag LABEL X1 Y1 X2 Y2 | sleep S | shot NAME | # comment
import sys, time
import Quartz
from AppKit import NSRunningApplication
NSApplicationActivateIgnoringOtherApps = 2  # AppKit constant (10.6 PyObjC lacks the name)

def windows():
    for w in Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID):
        if w.get('kCGWindowOwnerName') in ('Panel', 'M-Audio Firewire'):
            b = w['kCGWindowBounds']
            print w['kCGWindowNumber'], int(b['X']), int(b['Y']), int(b['Width']), int(b['Height']), w.get('kCGWindowName', '').encode('utf8')

def click(x, y):
    p = (float(x), float(y))
    for t in (Quartz.kCGEventMouseMoved, Quartz.kCGEventLeftMouseDown, Quartz.kCGEventLeftMouseUp):
        e = Quartz.CGEventCreateMouseEvent(None, t, p, Quartz.kCGMouseButtonLeft)
        Quartz.CGEventPost(Quartz.kCGHIDEventTap, e)
        time.sleep(0.08)

def shot(path):
    from AppKit import NSBitmapImageRep, NSPNGFileType
    for w in Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID):
        if w.get('kCGWindowOwnerName') in ('Panel', 'M-Audio Firewire'):
            img = Quartz.CGWindowListCreateImage(Quartz.CGRectNull, Quartz.kCGWindowListOptionIncludingWindow,
                                                 w['kCGWindowNumber'], Quartz.kCGWindowImageBoundsIgnoreFraming)
            rep = NSBitmapImageRep.alloc().initWithCGImage_(img)
            rep.representationUsingType_properties_(NSPNGFileType, None).writeToFile_atomically_(path, True)
            print path
            return

def origin():
    for w in Quartz.CGWindowListCopyWindowInfo(Quartz.kCGWindowListOptionOnScreenOnly, Quartz.kCGNullWindowID):
        if w.get('kCGWindowOwnerName') in ('Panel', 'M-Audio Firewire'):
            return int(w['kCGWindowBounds']['X']), int(w['kCGWindowBounds']['Y'])

def drag(x1, y1, x2, y2):
    post = lambda t, p: Quartz.CGEventPost(Quartz.kCGHIDEventTap, Quartz.CGEventCreateMouseEvent(None, t, p, Quartz.kCGMouseButtonLeft))
    post(Quartz.kCGEventMouseMoved, (x1, y1)); time.sleep(0.08)
    post(Quartz.kCGEventLeftMouseDown, (x1, y1)); time.sleep(0.15)
    for i in range(1, 11):
        post(Quartz.kCGEventLeftMouseDragged, (x1 + (x2 - x1) * i / 10.0, y1 + (y2 - y1) * i / 10.0)); time.sleep(0.04)
    post(Quartz.kCGEventLeftMouseUp, (x2, y2)); time.sleep(0.1)

def seq(path, shotdir='.'):
    ox, oy = origin()
    for line in open(path):
        line = line.strip()
        if not line or line.startswith('#'): continue
        a = line.split()
        print int(time.time() * 1000), line
        sys.stdout.flush()
        if a[0] == 'click': click(ox + float(a[2]), oy + float(a[3])); time.sleep(1.2)
        elif a[0] == 'drag': drag(ox + float(a[2]), oy + float(a[3]), ox + float(a[4]), oy + float(a[5])); time.sleep(1.2)
        elif a[0] == 'sleep': time.sleep(float(a[1]))
        elif a[0] == 'shot': shot('%s/%s.png' % (shotdir, a[1]))

def front():
    for a in NSRunningApplication.runningApplicationsWithBundleIdentifier_('com.m-audio.appc.MAfw') or []:
        a.activateWithOptions_(NSApplicationActivateIgnoringOtherApps)

if __name__ == '__main__':
    c = sys.argv[1]
    if c == 'windows': windows()
    elif c == 'click': click(sys.argv[2], sys.argv[3])
    elif c == 'front': front()
    elif c == 'shot': shot(sys.argv[2])
    elif c == 'seq': seq(sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else '.')
