// m_audio_b_FWAudioDevice::SetStaticPropertyAndNotifyUserClients
// Entry: 00002928
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWAudioDevice::SetStaticPropertyAndNotifyUserClients(char const*, unsigned int) */

void __thiscall
m_audio_b_FWAudioDevice::SetStaticPropertyAndNotifyUserClients
          (m_audio_b_FWAudioDevice *this,char *param_1,uint param_2)

{
  OSObject *pOVar1;
  
  if (*(long *)(this + 0xf8) != 0) {
    pOVar1 = (OSObject *)OSNumber::withNumber((ulong)param_2,0x20);
    if (pOVar1 != (OSObject *)0x0) {
      (**(code **)(*(long *)this + 0x268))(this,param_1,pOVar1);
      m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients
                (*(m_audio_b_FWPropertyDict **)(this + 0xf8),param_1,pOVar1);
                    /* WARNING: Could not recover jumptable at 0x00002998. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*(long *)pOVar1 + 0x28))(pOVar1);
      return;
    }
  }
  return;
}
