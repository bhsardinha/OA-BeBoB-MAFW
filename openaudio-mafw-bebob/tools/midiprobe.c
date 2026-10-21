// midiprobe - CoreMIDI bench tool for M-H2/M-V: lists devices/endpoints, sends bytes to a destination, listens on all sources.
//   midiprobe list
//   midiprobe send <dest-index> <hex bytes...>        (a SysEx F0..F7 is sent as one MIDISysexSendRequest)
//   midiprobe removedev <device-index>
//   midiprobe listen <seconds>                        (prints every received byte string with a host-time stamp)
//   midiprobe loop <dest-index> <seconds>             (sends a note-on/off pair and a SysEx every second, listens meanwhile: DIN cable out -> in)
// Build: cc -o midiprobe midiprobe.c -framework CoreMIDI -framework CoreFoundation -framework CoreAudio
#include <CoreMIDI/CoreMIDI.h>
#include <CoreAudio/HostTime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void name(MIDIObjectRef o, CFStringRef prop, char *out, size_t n)
{
    CFStringRef s = NULL;
    out[0] = 0;
    if (MIDIObjectGetStringProperty(o, prop, &s) == noErr && s) { CFStringGetCString(s, out, n, kCFStringEncodingUTF8); CFRelease(s); }
}

static void list(void)
{
    ItemCount nd = MIDIGetNumberOfDevices(), ns = MIDIGetNumberOfSources(), nt = MIDIGetNumberOfDestinations();
    printf("devices %lu, sources %lu, destinations %lu\n", nd, ns, nt);
    for (ItemCount i = 0; i < nd; i++) {
        MIDIDeviceRef d = MIDIGetDevice(i);
        char nm[128], mf[128], dr[128];
        SInt32 off = 0;
        name(d, kMIDIPropertyName, nm, sizeof nm); name(d, kMIDIPropertyManufacturer, mf, sizeof mf); name(d, kMIDIPropertyDriverOwner, dr, sizeof dr);
        MIDIObjectGetIntegerProperty(d, kMIDIPropertyOffline, &off);
        char im[256];
        name(d, kMIDIPropertyImage, im, sizeof im);
        printf("device %lu \"%s\" manufacturer \"%s\" driver \"%s\"%s image \"%s\"\n", i, nm, mf, dr, off ? " [offline]" : "", im);
        for (ItemCount e = 0; e < MIDIDeviceGetNumberOfEntities(d); e++) {
            MIDIEntityRef en = MIDIDeviceGetEntity(d, e);
            name(en, kMIDIPropertyName, nm, sizeof nm);
            printf("   entity \"%s\": %lu in-sources, %lu out-destinations\n", nm, MIDIEntityGetNumberOfSources(en), MIDIEntityGetNumberOfDestinations(en));
        }
    }
    for (ItemCount i = 0; i < ns; i++) { char nm[128]; name(MIDIGetSource(i), kMIDIPropertyName, nm, sizeof nm); printf("source %lu \"%s\"\n", i, nm); }
    for (ItemCount i = 0; i < nt; i++) { char nm[128]; name(MIDIGetDestination(i), kMIDIPropertyName, nm, sizeof nm); printf("destination %lu \"%s\"\n", i, nm); }
}

static void readProc(const MIDIPacketList *pl, void *rc, void *sc)
{
    (void)rc; (void)sc;
    const MIDIPacket *p = &pl->packet[0];
    for (UInt32 i = 0; i < pl->numPackets; i++) {
        printf("rx %12llu ns:", (unsigned long long)AudioConvertHostTimeToNanos(p->timeStamp));
        for (UInt16 b = 0; b < p->length; b++) printf(" %02X", p->data[b]);
        printf("\n");
        p = MIDIPacketNext(p);
    }
    fflush(stdout);
}

static MIDIClientRef gClient;
static MIDIPortRef gIn, gOut;

static void setup(void)
{
    MIDIClientCreate(CFSTR("midiprobe"), NULL, NULL, &gClient);
    MIDIInputPortCreate(gClient, CFSTR("in"), readProc, NULL, &gIn);
    MIDIOutputPortCreate(gClient, CFSTR("out"), &gOut);
    for (ItemCount i = 0; i < MIDIGetNumberOfSources(); i++) MIDIPortConnectSource(gIn, MIDIGetSource(i), NULL);
}

static Byte sysexBuf[512];
static MIDISysexSendRequest sysexReq;
static void sysexDone(MIDISysexSendRequest *r) { printf("sysex sent (%u bytes unsent)\n", (unsigned)r->bytesToSend); fflush(stdout); }

static OSStatus sendBytes(MIDIEndpointRef dest, const Byte *b, size_t n)
{
    if (b[0] == 0xF0) {
        memcpy(sysexBuf, b, n);
        sysexReq = (MIDISysexSendRequest){ dest, sysexBuf, (UInt32)n, 0, {0, 0, 0}, sysexDone, NULL };
        return MIDISendSysex(&sysexReq);
    }
    Byte buf[1024];
    MIDIPacketList *pl = (MIDIPacketList *)buf;
    MIDIPacket *p = MIDIPacketListInit(pl);
    MIDIPacketListAdd(pl, sizeof buf, p, 0, n, b);
    return MIDISend(gOut, dest, pl);
}

static Byte hex(const char *s) { return (Byte)strtol(s, NULL, 16); }

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: midiprobe list | send <dest> <hex..> | listen <s> | loop <dest> <s>\n"); return 2; }
    if (!strcmp(argv[1], "list")) { list(); return 0; }
    if (!strcmp(argv[1], "removedev") && argc >= 3) {          // drop a (stale) device from the MIDI setup; the driver re-creates it on the next start
        OSStatus s = MIDISetupRemoveDevice(MIDIGetDevice((ItemCount)atoi(argv[2])));
        printf("remove status %d\n", (int)s);
        return s != noErr;
    }
    setup();
    if (!strcmp(argv[1], "listen") && argc >= 3) { sleep((unsigned)atoi(argv[2])); return 0; }
    if (!strcmp(argv[1], "send") && argc >= 4) {
        MIDIEndpointRef d = MIDIGetDestination((ItemCount)atoi(argv[2]));
        Byte b[512]; size_t n = 0;
        for (int i = 3; i < argc && n < sizeof b; i++) b[n++] = hex(argv[i]);
        OSStatus s = sendBytes(d, b, n);
        printf("send status %d\n", (int)s);
        sleep(2);
        return s != noErr;
    }
    if (!strcmp(argv[1], "loop") && argc >= 4) {
        MIDIEndpointRef d = MIDIGetDestination((ItemCount)atoi(argv[2]));
        int secs = atoi(argv[3]);
        const Byte on[] = { 0x90, 0x3C, 0x64 }, off[] = { 0x80, 0x3C, 0x00 };
        const Byte sx[] = { 0xF0, 0x7D, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0xF7 };
        for (int t = 0; t < secs; t++) {
            sendBytes(d, on, 3); usleep(100000); sendBytes(d, off, 3); usleep(100000); sendBytes(d, sx, sizeof sx);
            sleep(1);
        }
        return 0;
    }
    return 2;
}
