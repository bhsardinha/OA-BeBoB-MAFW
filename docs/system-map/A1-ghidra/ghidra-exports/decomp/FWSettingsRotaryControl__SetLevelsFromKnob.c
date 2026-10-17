// FWSettingsRotaryControl::SetLevelsFromKnob
// Entry: 00021284
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsRotaryControl::SetLevelsFromKnob(unsigned long long, char const*, unsigned int,
   m_audio_FWb::BaseChannelGroup*, int) */

void __thiscall
FWSettingsRotaryControl::SetLevelsFromKnob
          (FWSettingsRotaryControl *this,ulonglong param_1,char *param_2,uint param_3,
          BaseChannelGroup *param_4,int param_5)

{
  OSArray *pOVar1;
  uint uVar2;
  long lVar3;
  OSArray *local_38;
  
  if (param_4 != (BaseChannelGroup *)0x0) {
    local_38 = (OSArray *)0x0;
    uVar2 = (**(code **)(*(long *)param_4 + 0x10))();
    if (param_3 < (uVar2 >> 1) + param_3) {
      lVar3 = 0;
      do {
        if ((param_1 >> ((ulong)param_3 + lVar3 & 0x3f) & 1) != 0) {
          SetPairLevelsFromRotary(this,param_2,(uint)lVar3,&local_38,param_5);
        }
        pOVar1 = local_38;
        lVar3 = lVar3 + 1;
      } while ((ulong)((uVar2 >> 1) - 1) + 1 != lVar3);
      if (local_38 != (OSArray *)0x0) {
        m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients
                  (*(m_audio_b_FWPropertyDict **)(this + 0xc0),param_2,(OSObject *)local_38);
        (**(code **)(*(long *)pOVar1 + 0x28))(pOVar1);
      }
    }
  }
  return;
}
