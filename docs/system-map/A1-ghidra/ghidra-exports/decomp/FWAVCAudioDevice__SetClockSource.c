// m_audio_b_FWAVCAudioDevice::SetClockSource
// Entry: 00023858
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAVCAudioDevice::SetClockSource(unsigned int) */

int __thiscall
m_audio_b_FWAVCAudioDevice::SetClockSource(m_audio_b_FWAVCAudioDevice *this,uint param_1)

{
  byte bVar1;
  long *plVar2;
  int iVar3;
  bool bVar4;
  
  plVar2 = *(long **)(this + 0xe8);
  if (plVar2 != (long *)0x0) {
    (**(code **)(*plVar2 + 0xa50))(plVar2);
  }
  if (param_1 < 0x40000000) {
    _IOLog("M-FW: Converting old digital bitmap (%X)\n",param_1);
    switch(param_1) {
    case 0:
      param_1 = 0x40000000;
      break;
    case 1:
      param_1 = 0x40000004;
      break;
    case 2:
      param_1 = 0x40000001;
      break;
    case 3:
      param_1 = 0x40000005;
    }
  }
  if (*(long *)(this + 0xe8) == 0) {
    iVar3 = 0;
  }
  else {
    bVar1 = *(byte *)(*(long *)(this + 0xe8) + 0x2d0);
    iVar3 = (**(code **)(*(long *)this + 0xb60))(this,param_1,0);
    if (iVar3 == 0) {
      if ((int)param_1 < 0) {
        bVar4 = (param_1 & 0xf) != 0;
      }
      else {
        bVar4 = (bool)(-((param_1 & 3) - 1 < 2) & 1);
      }
      if (bVar4 != (bool)bVar1) {
        (**(code **)(**(long **)(this + 0xe8) + 0xce8))(*(long **)(this + 0xe8),bVar4);
      }
      m_audio_b_FWBaseEngine::CompletePendingStateChanges(*(m_audio_b_FWBaseEngine **)(this + 0xe8))
      ;
    }
  }
  if (plVar2 != (long *)0x0) {
    (**(code **)(*plVar2 + 0xa58))(plVar2);
  }
  return iVar3;
}
