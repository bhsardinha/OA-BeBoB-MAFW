// FWSettingsLevels::SetSWReturnLevel
// Entry: 00022396
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::SetSWReturnLevel(unsigned int, unsigned int) */

undefined8 __thiscall
FWSettingsLevels::SetSWReturnLevel(FWSettingsLevels *this,uint param_1,uint param_2)

{
  uint uVar1;
  undefined8 uVar2;
  undefined4 local_30;
  uchar local_2c;
  uchar local_2b;
  undefined4 local_28;
  
  local_30 = 3;
  local_2c = '\0';
  local_2b = (uchar)param_1;
  local_28 = 0;
  if (((*(long **)(this + 0x8b0) != (long *)0x0) && (param_1 != 0)) &&
     (uVar1 = (**(code **)(**(long **)(this + 0x8b0) + 0x10))(), param_1 <= uVar1)) {
    uVar2 = (**(code **)(**(long **)(this + 0xa58) + 0xb48))(*(long **)(this + 0xa58),&local_30);
    if ((int)uVar2 != 0) {
      return uVar2;
    }
    uVar2 = m_audio_b_FWAVCAudioDevice::AVCControlFeatureFunctionBlock
                      (*(m_audio_b_FWAVCAudioDevice **)(this + 0xa58),local_2c,'\x02',local_2b,
                       (ushort)(param_2 << 7));
    return uVar2;
  }
  return 0xe00002c2;
}
