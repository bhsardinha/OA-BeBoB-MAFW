// com_m_audio_FWUserClient::getTargetAndMethodForIndex
// Entry: 000072f6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::getTargetAndMethodForIndex(IOService**, unsigned int) */

undefined ** __thiscall
com_m_audio_FWUserClient::getTargetAndMethodForIndex
          (com_m_audio_FWUserClient *this,IOService **param_1,uint param_2)

{
  if ((*(long *)(this + 0x290) != 0) && (param_2 - 500 < 8)) {
    *param_1 = (IOService *)this;
    return &sMethods + (ulong)(param_2 - 500) * 6;
  }
  return (undefined **)0x0;
}
