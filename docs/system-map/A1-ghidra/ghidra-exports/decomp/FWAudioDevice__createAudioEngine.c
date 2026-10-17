// m_audio_b_FWAudioDevice::createAudioEngine
// Entry: 00002fd8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e10)

/* m_audio_b_FWAudioDevice::createAudioEngine() */

undefined8 __thiscall m_audio_b_FWAudioDevice::createAudioEngine(m_audio_b_FWAudioDevice *this)

{
  char cVar1;
  int iVar2;
  IOAudioEngine *pIVar3;
  long lVar4;
  
  (**(code **)(*(long *)this + 0x988))(this);
  *(undefined8 *)(this + 0xe8) = 0;
  pIVar3 = (IOAudioEngine *)(**(code **)(*(long *)this + 0xa20))(this);
  if (pIVar3 != (IOAudioEngine *)0x0) {
    cVar1 = (**(code **)(*(long *)pIVar3 + 0x88))(pIVar3);
    if (cVar1 == '\0') {
      (**(code **)(*(long *)pIVar3 + 0x90))(pIVar3);
    }
    else {
      (**(code **)(*(long *)this + 0xa38))(this,pIVar3);
      SetChannelNamesForMainEngine(this,pIVar3);
      iVar2 = (**(code **)(*(long *)this + 0x978))(this,pIVar3);
      if (iVar2 == 0) {
        lVar4 = (**(code **)(*(long *)pIVar3 + 0xb30))(pIVar3);
        if (lVar4 != 0) {
          (**(code **)(*(long *)this + 0x268))(this,"MAMainEngineUniqueID",lVar4);
        }
        *(IOAudioEngine **)(this + 0xe8) = pIVar3;
        (**(code **)(*(long *)pIVar3 + 0x28))(pIVar3);
        return 1;
      }
      (**(code **)(*(long *)pIVar3 + 0x28))(pIVar3);
    }
  }
  return 0;
}
