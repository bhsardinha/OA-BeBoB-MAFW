// m_audio_b_FWBaseArrayProp::ChangedValueThroughAudioControl
// Entry: 000082ea
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWBaseArrayProp::ChangedValueThroughAudioControl(IOAudioControl*, int, int) */

int __thiscall
m_audio_b_FWBaseArrayProp::ChangedValueThroughAudioControl
          (m_audio_b_FWBaseArrayProp *this,IOAudioControl *param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  long *plVar3;
  uint uVar4;
  int local_3c;
  OSObject *local_38;
  
  local_3c = SeriatimValuesToOSArray(this,*(uint *)(this + 0x2c),&local_38);
  if (local_3c == 0) {
    iVar1 = (**(code **)(*(long *)param_1 + 0x9b0))(param_1);
    plVar3 = (long *)OSNumber::withNumber((long)param_3,0x20);
    if (plVar3 == (long *)0x0) {
      local_3c = -0x1ffffd43;
    }
    else {
      if (iVar1 == 0) {
        if (*(int *)(this + 0x2c) != 0) {
          uVar4 = 0;
          do {
            (**(code **)(*(long *)local_38 + 0x1d0))(local_38,uVar4,plVar3);
            uVar4 = uVar4 + 1;
          } while (uVar4 < *(uint *)(this + 0x2c));
        }
      }
      else {
        uVar2 = (**(code **)(*(long *)param_1 + 0x9a8))(param_1);
        (**(code **)(*(long *)local_38 + 0x1d0))(local_38,uVar2,plVar3);
      }
      if (*(m_audio_b_FWPropertyDict **)(this + 0x20) != (m_audio_b_FWPropertyDict *)0x0) {
        m_audio_b_FWPropertyDict::InjectModifiedProperty
                  (*(m_audio_b_FWPropertyDict **)(this + 0x20),*(OSSymbol **)(this + 0x18),local_38,
                   false);
      }
      (**(code **)(*plVar3 + 0x28))(plVar3);
    }
    (**(code **)(*(long *)local_38 + 0x28))(local_38);
  }
  return local_3c;
}
