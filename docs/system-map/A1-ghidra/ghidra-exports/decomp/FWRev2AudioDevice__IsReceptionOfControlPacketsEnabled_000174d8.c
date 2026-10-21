// FWRev2AudioDevice::IsReceptionOfControlPacketsEnabled
// Entry: 000174d8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::IsReceptionOfControlPacketsEnabled() */

bool __thiscall FWRev2AudioDevice::IsReceptionOfControlPacketsEnabled(FWRev2AudioDevice *this)

{
  return *(long *)(this + 0x2c8) != 0;
}
