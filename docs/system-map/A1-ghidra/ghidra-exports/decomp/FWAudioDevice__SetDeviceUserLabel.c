// m_audio_b_FWAudioDevice::SetDeviceUserLabel
// Entry: 0000269e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAudioDevice::SetDeviceUserLabel(OSObject*) */

int __thiscall
m_audio_b_FWAudioDevice::SetDeviceUserLabel(m_audio_b_FWAudioDevice *this,OSObject *param_1)

{
  int iVar1;
  OSObject *pOVar2;
  char *pcVar3;
  
  pOVar2 = (OSObject *)
           OSMetaClassBase::safeMetaCast
                     ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030090);
  if (pOVar2 == (OSObject *)0x0) {
    iVar1 = -0x1ffffd3e;
  }
  else {
    pcVar3 = (char *)(**(code **)(*(long *)pOVar2 + 0x148))(pOVar2);
    iVar1 = SetDeviceLabel(this,pcVar3);
    if (iVar1 == 0) {
      m_audio_b_FWPropertyDict::SetPropertyAndNotifyUserClients
                (*(m_audio_b_FWPropertyDict **)(this + 0xf8),"MADeviceUserLabel",pOVar2);
    }
  }
  return iVar1;
}
