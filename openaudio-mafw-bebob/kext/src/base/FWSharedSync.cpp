#include "FWSharedSync.h"

bool TRingBuffer::InitializeWithSizeInBytes(UInt32 bytes)
{
    Invalidate();
    size = bytes;
    mem = IOBufferMemoryDescriptor::withOptions(0, bytes, 0x1000);
    if (!mem) return false;
    if (mem->prepare() != kIOReturnSuccess) { mem->release(); mem = 0; return false; }
    data = static_cast<UInt8 *>(mem->getBytesNoCopy());
    if (!data) { mem->complete(); mem->release(); mem = 0; return false; }
    bzero(data, size);
    readOff = writeOff = readWraps = writeWraps = 0;
    readWrapped = writeWrapped = false;
    return true;
}

void TRingBuffer::Invalidate()
{
    if (mem) {
        size = 0;
        data = 0;
        mem->complete();
        mem->release();
        mem = 0;
    }
}

bool TRingBuffer::AdvanceIndex(UInt32 n, UInt32 &idx, UInt32 &wraps) const
{
    UInt32 v = idx + n;
    idx = v;
    if (size <= v) {
        idx = v - size;
        wraps++;
        return true;
    }
    return false;
}

void TRingBuffer::WriteNextData(UInt32 n, const void *src)
{
    UInt32 old = writeOff, v = old + n;
    writeOff = v;
    UInt32 len = n;
    if (v < size) {
        writeWrapped = false;
    } else {
        UInt32 spill = v - size;
        writeOff = spill;
        writeWraps++;
        writeWrapped = true;
        if (v != size) {
            len = n - spill;
            bcopy(static_cast<const UInt8 *>(src) + len, data, spill);
        }
    }
    bcopy(src, data + old, len);
}

void TRingBuffer::ReadNextData(UInt32 n, void *dst)
{
    UInt32 old = readOff, v = old + n;
    readOff = v;
    UInt32 len = n;
    if (v < size) {
        readWrapped = false;
    } else {
        UInt32 spill = v - size;
        readOff = spill;
        readWraps++;
        readWrapped = true;
        if (v != size) {
            len = n - spill;
            bcopy(data, static_cast<UInt8 *>(dst) + len, spill);
        }
    }
    bcopy(data + old, dst, len);
}
