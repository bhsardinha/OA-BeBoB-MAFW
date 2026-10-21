// FWRev2AudioDevice::UpdatePeakMeters
// Entry: 00012aae
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* FWRev2AudioDevice::UpdatePeakMeters(unsigned int*, unsigned int) */

void __thiscall
FWRev2AudioDevice::UpdatePeakMeters(FWRev2AudioDevice *this,uint *param_1,uint param_2)

{
  ulong uVar1;
  
  if (param_2 != 0) {
    uVar1 = 0;
    do {
      *(uint *)(*(long *)(this + 0x108) + uVar1 * 4) =
           (param_1[uVar1] & 0xff00) * 0x100 + param_1[uVar1] * 0x1000000 >> 0x10;
      uVar1 = uVar1 + 1;
    } while (param_2 != uVar1);
  }
  return;
}
