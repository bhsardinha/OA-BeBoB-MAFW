// m_audio_b_FWAVCAudioDevice::SendCommandControlLEDTest
// Entry: 0002328a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* m_audio_b_FWAVCAudioDevice::SendCommandControlLEDTest() */

void __thiscall
m_audio_b_FWAVCAudioDevice::SendCommandControlLEDTest(m_audio_b_FWAVCAudioDevice *this)

{
  undefined1 *puVar1;
  undefined8 in_R9;
  undefined1 *puVar2;
  undefined4 uVar3;
  undefined1 local_3c [12];
  undefined1 *local_30;
  uint local_28;
  undefined1 local_24 [16];
  undefined4 local_14;
  
  local_28 = 8;
  local_30 = local_24;
  _bzero(local_30,8);
  uVar3 = 8;
  puVar2 = local_3c;
  _bzero(puVar2,8);
  *local_30 = 0;
  puVar1 = local_30;
  if (1 < local_28) {
    puVar1 = local_30 + 1;
  }
  *puVar1 = 0xff;
  puVar1 = local_30;
  if (2 < local_28) {
    puVar1 = local_30 + 2;
  }
  *puVar1 = 0;
  puVar1 = local_30;
  if (3 < local_28) {
    puVar1 = local_30 + 3;
  }
  *puVar1 = 2;
  puVar1 = local_30;
  if (4 < local_28) {
    puVar1 = local_30 + 4;
  }
  *puVar1 = 0;
  puVar1 = local_30;
  if (5 < local_28) {
    puVar1 = local_30 + 5;
  }
  *puVar1 = 0;
  puVar1 = local_30;
  if (6 < local_28) {
    puVar1 = local_30 + 6;
  }
  *puVar1 = 0;
  puVar1 = local_30;
  if (7 < local_28) {
    puVar1 = local_30 + 7;
  }
  *puVar1 = 0;
  local_14 = 8;
  (**(code **)(**(long **)(this + 0x268) + 0x8b8))
            (*(long **)(this + 0x268),local_30,8,puVar2,&local_14,in_R9,puVar2,uVar3);
  return;
}
