// com_m_audio_FWEngine2::initHardware
// Entry: 00012622
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::initHardware(IOService*) */

char __thiscall com_m_audio_FWEngine2::initHardware(com_m_audio_FWEngine2 *this,IOService *param_1)

{
  char cVar1;
  int iVar2;
  m_audio_b_FWDCLOutputProgram *this_00;
  m_audio_b_FWDCLInputProgram *this_01;
  m_audio_b_FWAudioDevice *pmVar3;
  
  cVar1 = (*(code *)PTR_initHardware_000333f0)(this);
  if (cVar1 != '\0') {
    this_00 = OSObject::operator_new((OSObject *)&DAT_00000620,(ulong)param_1);
    if (this_00 == (m_audio_b_FWDCLOutputProgram *)0x0) {
      *(undefined8 *)(this + 0x350) = 0;
      return '\0';
    }
    m_audio_b_FWDCLOutputProgram::m_audio_b_FWDCLOutputProgram(this_00);
    *(m_audio_b_FWDCLOutputProgram **)(this + 0x350) = this_00;
    pmVar3 = *(m_audio_b_FWAudioDevice **)(this + 0x138);
    iVar2 = m_audio_b_FWDCLProgram::Initialize
                      ((m_audio_b_FWDCLProgram *)this_00,pmVar3,
                       (FWDCLSharedSyncData *)(this + 0x240));
    if (iVar2 != 0) goto LAB_0001269a;
    this_01 = OSObject::operator_new((OSObject *)&DAT_00000518,(ulong)pmVar3);
    if (this_01 == (m_audio_b_FWDCLInputProgram *)0x0) {
      *(undefined8 *)(this + 0x358) = 0;
      goto LAB_0001269a;
    }
    m_audio_b_FWDCLInputProgram::m_audio_b_FWDCLInputProgram(this_01);
    *(m_audio_b_FWDCLInputProgram **)(this + 0x358) = this_01;
    iVar2 = m_audio_b_FWDCLProgram::Initialize
                      ((m_audio_b_FWDCLProgram *)this_01,*(m_audio_b_FWAudioDevice **)(this + 0x138)
                       ,(FWDCLSharedSyncData *)(this + 0x240));
    if (iVar2 == 0) {
      *(undefined8 *)(*(long *)(this + 0x358) + 0x508) = *(undefined8 *)(this + 0x350);
      *(undefined1 *)(*(long *)(this + 0x358) + 0x8e) = 1;
      *(undefined1 *)(*(long *)(this + 0x350) + 0x8e) = 1;
      iVar2 = m_audio_b_FWBaseEngine::AddDCLProgram
                        ((m_audio_b_FWBaseEngine *)this,*(m_audio_b_FWDCLProgram **)(this + 0x350));
      if ((iVar2 == 0) &&
         (iVar2 = m_audio_b_FWBaseEngine::AddDCLProgram
                            ((m_audio_b_FWBaseEngine *)this,
                             *(m_audio_b_FWDCLProgram **)(this + 0x358)), iVar2 == 0)) {
        return cVar1;
      }
    }
  }
  (**(code **)(**(long **)(this + 0x358) + 0x28))();
LAB_0001269a:
  (**(code **)(**(long **)(this + 0x350) + 0x28))();
  return '\0';
}
