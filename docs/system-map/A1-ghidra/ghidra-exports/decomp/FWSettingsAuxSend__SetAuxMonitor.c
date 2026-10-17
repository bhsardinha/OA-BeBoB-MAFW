// FWSettingsAuxSend::SetAuxMonitor
// Entry: 00020c68
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::SetAuxMonitor(unsigned int, unsigned int) */

undefined8 __thiscall
FWSettingsAuxSend::SetAuxMonitor(FWSettingsAuxSend *this,uint param_1,uint param_2)

{
  undefined8 uVar1;
  undefined4 local_28;
  uchar local_24;
  undefined1 local_23;
  undefined4 local_20;
  
  local_28 = 5;
  local_24 = '\0';
  local_23 = (undefined1)param_1;
  local_20 = 0;
  if (param_2 < 2) {
    uVar1 = (**(code **)(**(long **)(this + 0x2d0) + 0xb48))(*(long **)(this + 0x2d0),&local_28);
    if ((int)uVar1 == 0) {
      uVar1 = m_audio_b_FWAVCAudioDevice::AVCSelectorCommand
                        (*(m_audio_b_FWAVCAudioDevice **)(this + 0x2d0),local_24,param_2 != 0);
    }
  }
  else {
    uVar1 = 0xe00002c2;
  }
  return uVar1;
}
