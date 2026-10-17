// _CPULoad
// Entry: 000227d1
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

uint _CPULoad(char param_1)

{
  long lVar1;
  ulong uVar2;
  
  if (param_1 == '\x01') {
    lVar1 = 0;
    do {
      *(undefined8 *)((long)&_x_2396 + lVar1) = *(undefined8 *)((long)&_orig_x_2397 + lVar1);
      lVar1 = lVar1 + 8;
    } while (lVar1 != 200);
  }
  else {
    if (_k_2395 != 0x19) goto LAB_00022895;
    lVar1 = 7;
    do {
      uVar2 = *(ulong *)(lVar1 * 8 + 0x4b538);
      *(ulong *)(lVar1 * 8 + 0x4b538) = uVar2 >> 1 ^ (&_x_2396)[lVar1] ^ (&_mag01_2398)[uVar2 & 1];
      lVar1 = lVar1 + 1;
    } while (lVar1 != 0x19);
    lVar1 = 0;
    do {
      (&DAT_0004b600)[lVar1] =
           (ulong)(&DAT_0004b600)[lVar1] >> 1 ^ (&_x_2396)[lVar1] ^
           (&_mag01_2398)[(&DAT_0004b600)[lVar1] & 1];
      lVar1 = lVar1 + 1;
    } while (lVar1 != 7);
  }
  _k_2395 = 0;
LAB_00022895:
  lVar1 = (long)_k_2395;
  _k_2395 = _k_2395 + 1;
  uVar2 = ((&_x_2396)[lVar1] & 0x56b64a) << 7 ^ (&_x_2396)[lVar1];
  uVar2 = (ulong)((uint)(uVar2 << 0xf) & 0xdb8b0000) ^ uVar2 & 0xffffffff;
  return (uint)(uVar2 >> 0x10) ^ (uint)uVar2;
}
